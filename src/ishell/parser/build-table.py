import sys

try:
    import pandas as pd
except ModuleNotFoundError:
    print("Install 'pandas' with 'python3 -m pip install pandas'", file=sys.stderr)
    exit(1)
df = None
filename = sys.argv[1]
try:
    df = pd.read_excel(filename, engine="odf")
except ImportError:
    print("Install 'odfpy' with 'python3 -m pip install odfpy'", file=sys.stderr)
    exit(1)

# find first row with 'TABLE'
x = df.loc[df.iloc[:, 0] == "TABLE"]
top_of_input_row = int(x.index.values[0]) + 1
top_of_stack_col = 1
n_elms = len(df.iloc[top_of_input_row + 1 :, top_of_stack_col])
token_names = list(df.iloc[top_of_input_row + 1 :, top_of_stack_col])
table = pd.DataFrame(
    df.iloc[
        top_of_input_row + 1 :, top_of_stack_col + 1 : top_of_stack_col + 1 + n_elms
    ]
).reset_index(drop=True)
table.columns = token_names
# token_names = list(token_names)

# print(table)
# print(token_names)

precedences = ["TAKE", "YIELD", "SAME", "ERR", "ACC"]
largest_width = max([len(h) for h in token_names] + [len(p) for p in precedences])
cell_width = largest_width + len("Precedence::")


def pad_str(s):
    return f"{s:{cell_width}}"


def elm_to_str(elm):
    p = "ERR"
    if elm == ">":
        p = "TAKE"
    elif elm == "<":
        p = "YIELD"
    elif elm == "=":
        p = "SAME"
    elif elm == "ACC":
        p = "ACC"
    return f"Precedence::{p}"


print("namespace precedence_table {")

print(
    f"std::array<std::array<Precedence, {n_elms}>, {n_elms}> table = {{ std::array<Precedence, {n_elms}>"
)
header = ",".join([pad_str(h) for h in token_names])
print(f"  /*  {pad_str('Top Of Input ->')}  {header}*/")
for ridx, row in table.iterrows():
    elms = ",".join([pad_str(elm_to_str(elm)) for cidx, elm in row.items()])
    print(f"  /*{pad_str(token_names[ridx])}*/ {{", end="")
    print(elms, end="")
    print("},")
print("};")

print("unsigned getIndex(TokenType tt) {")
print("switch (tt) {")
for idx, t in enumerate(token_names):
    print(f"case TokenType::{t}: return {idx};")
print(f"default: return {n_elms-1};")
print("}")
print("}")


print("Precedence getAction(TokenType top_of_stack, TokenType top_of_input) {")
print("return table[getIndex(top_of_stack)][getIndex(top_of_input)];")
print("}")

print("}")
