import os, numpy as np, gc

# dimensiunile listelor de generat
dimensiuni = [100, 10_000, 100_000, 400_000, 1_000_000, 10_000_000]

# tipurile de liste cerute
tipuri = ["aleator", "sortat", "inversat", "aproape_sortat", "plat"]

# directorul unde vom scrie fisierele CSV
os.makedirs("liste", exist_ok=True)

# --- generatoare pentru fiecare tip ---

def aleator(n):
    # valori intregi aleatoare in intervalul [0, 1e9)
    return np.random.randint(0, 1_000_000_000, n, np.int64)

def sortat(n):
    # sir crescator 0, 1, 2, ...
    return np.arange(n, dtype=np.int64)

def inversat(n):
    # sir descrescator n-1, n-2, ...
    return np.arange(n - 1, -1, -1, dtype=np.int64)

def aproape_sortat(n, p=0.05):
    # lista sortata cu p*100% pozitii incurcate
    a = np.arange(n, dtype=np.int64)
    k = int(n * p)
    if k:  # doar daca avem ce schimba
        i1 = np.random.choice(n, k, False)  # pozitii unice
        i2 = np.random.choice(n, k, True)   # pozitii (posibil) repetate
        a[i1], a[i2] = a[i2], a[i1]
    return a

def plat(n, distincte=20):
    # lista lunga cu putine valori distincte
    return np.random.choice(distincte, n).astype(np.int64)

# dictionar pentru acces rapid
gen = {
    "aleator": aleator,
    "sortat": sortat,
    "inversat": inversat,
    "aproape_sortat": aproape_sortat,
    "plat": plat,
}

# --- bucla principala: genereaza si scrie fisierele ---

for tip in tipuri:
    for n in dimensiuni:
        date = gen[tip](n)
        nume_fisier = f"liste/{tip}_{n}.csv"
        with open(nume_fisier, "w") as f:
            # scriem ca o singura coloana, un numar pe linie
            date.tofile(f, sep="\n", format="%d")
        # eliberam memoria pentru dimensiuni mari
        del date
        gc.collect()
