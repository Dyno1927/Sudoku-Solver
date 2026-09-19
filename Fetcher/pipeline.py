# Pipeline: websudoku.com -> my C solver -> websudoku.com.
# Fetches a puzzle, writes it as an 81-char string for Solver.c, reads the
# solved grid back, and (later) posts the answers to the site.
#
# Run from the repo root:  python Fetcher/pipeline.py [puzzle_id [level]]

import re
import subprocess
import sys
import urllib.parse
import urllib.request

LEVEL: int = 1  # 1=easy, 2=medium, 3=hard, 4=evil
BASE_URL: str = "http://west.websudoku.com/"

# Solver.c uses relative paths, so it must run from inside C/.
C_DIR: str = "C"


def build_url(args: list[str]) -> str:
    """Optional args: [puzzle_id] [level]. No id = random puzzle."""
    level: int = int(args[1]) if len(args) > 1 else LEVEL
    url: str = f"{BASE_URL}?level={level}"
    if args:
        url += f"&set_id={args[0]}"
    return url


def fetch(url: str) -> str:
    """GET the puzzle page, return the raw HTML string."""
    return urllib.request.urlopen(url).read().decode("utf-8")


def puzzle_string(page: str) -> str:
    """Pull the 81-char puzzle out of the page. '0' = empty cell, 1-9 = given."""
    cheat_match: re.Match[str] | None = re.search(r"var cheat='(\d{81})'", page)
    editmask_match: re.Match[str] | None = re.search(
        r'ID="editmask" [^>]* VALUE="([01]{81})"', page
    )

    # BuildSafety: Verify matches exist before extracting groups to prevent NoneType crashes
    if not cheat_match or not editmask_match:
        raise ValueError(
            "Could not find cheat or editmask patterns on the WebSudoku page."
        )

    cheat: str = cheat_match.group(1)
    editmask: str = editmask_match.group(1)

    return "".join(cheat[i] if editmask[i] == "1" else "0" for i in range(81))


def save_puzzle(puzzle: str) -> None:
    """Write puzzle to C/input/puzzle.txt so Solver.c can eat it."""
    with open(f"{C_DIR}/input/puzzle.txt", "w") as f:
        f.write(puzzle)


def solve() -> None:
    """Run the C solver; it writes C/output/solution.txt itself."""
    subprocess.run(["build/Solver"], cwd=C_DIR, check=True)


def solution_string() -> str:
    """Read C/output/solution.txt back, keep only the 81 digits."""
    with open(f"{C_DIR}/output/solution.txt", "r") as f:
        text: str = f.read()
    return re.sub(r"\D", "", text)


def place_answers(page: str, answer: str) -> str:
    """Post the solved grid to the site's form and return its verdict.

    The site compares posted cell values positionally in document order
    (its table is laid out column-first), so cell #i gets answer[i].
    """
    inputs: list[str] = re.findall(r"<INPUT([^>]*)>", page)

    data: dict[str, str] = {}
    cells: list[str] = []
    for tag in inputs:
        name_m: re.Match[str] | None = re.search(r"NAME=([^\s>]*)", tag)
        val_m: re.Match[str] | None = re.search(r'VALUE="([^"]*)"', tag)
        id_m: re.Match[str] | None = re.search(r"ID=f", tag)
        if id_m and name_m:
            data[name_m.group(1)] = val_m.group(1) if val_m else ""
            cells.append(name_m.group(1))
        elif name_m:
            hidden: str = name_m.group(1)
            if hidden not in (
                "submit",
                "pause",
                "printopts",
                "clear",
                "showopts",
                "jstimer",
            ):
                data[hidden] = val_m.group(1) if val_m else ""

    # positional fill: cell #i gets answer[i], in document order
    for pos, name in enumerate(cells):
        data[name] = answer[pos]
    data["submit"] = "How am I doing?"

    body: bytes = urllib.parse.urlencode(data).encode()
    resp: str = (
        urllib.request.urlopen(urllib.request.Request(BASE_URL, data=body))
        .read()
        .decode("utf-8")
    )

    verdict: re.Match[str] | None = re.search(
        r'<SPAN id="message">(.*?)</SPAN>', resp, re.DOTALL
    )
    if verdict:
        return re.sub(r"<[^>]+>", "", verdict.group(1))
    return "no verdict found"


def main() -> None:
    url: str = build_url(sys.argv[1:])
    page: str = fetch(url)

    puzzle: str = puzzle_string(page)
    save_puzzle(puzzle)

    solve()
    answer: str = solution_string()

    # The site leaked the real solution in `cheat` — free verification.
    cheat_match: re.Match[str] | None = re.search(r"var cheat='(\d{81})'", page)
    cheat: str = cheat_match.group(1) if cheat_match else ""

    print("puzzle:", puzzle)
    print("answer:", answer)
    print("solver vs site:", "MATCH" if answer == cheat else "MISMATCH")

    verdict: str = place_answers(page, answer)
    print("site verdict:", verdict)


if __name__ == "__main__":
    main()
