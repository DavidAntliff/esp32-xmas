import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize

cmaps = ('viridis', 'plasma', 'inferno', 'magma',
    'Greys', 'Purples', 'Blues', 'Greens', 'Oranges', 'Reds',
    'YlOrBr', 'YlOrRd', 'OrRd', 'PuRd', 'RdPu', 'BuPu',
    'GnBu', 'PuBu', 'YlGnBu', 'PuBuGn', 'BuGn', 'YlGn',
    'binary', 'gist_yarg', 'gist_gray', 'gray', 'bone', 'pink',
    'spring', 'summer', 'autumn', 'winter', 'cool', 'Wistia',
    'hot', 'afmhot', 'gist_heat', 'copper',
    'PiYG', 'PRGn', 'BrBG', 'PuOr', 'RdGy', 'RdBu',
    'RdYlBu', 'RdYlGn', 'Spectral', 'coolwarm', 'bwr', 'seismic',
    'Pastel1', 'Pastel2', 'Paired', 'Accent',
    'Dark2', 'Set1', 'Set2', 'Set3',
    'tab10', 'tab20', 'tab20b', 'tab20c',
    'flag', 'prism', 'ocean', 'gist_earth', 'terrain', 'gist_stern',
    'gnuplot', 'gnuplot2', 'CMRmap', 'cubehelix', 'brg', 'hsv',
    'gist_rainbow', 'rainbow', 'jet', 'nipy_spectral', 'gist_ncar')

R_BITS = 8
G_BITS = 8
B_BITS = 8
A_BITS = 8

NUM_STEPS = 256


def to_int(fval, bits):
    return int(round(((1 << bits) - 1) * fval))


def cmap_to_struct(cmap, num_steps, r_bits, g_bits, b_bits, a_bits):
    norm = Normalize(vmin=0, vmax=NUM_STEPS - 1)
    for i in range(num_steps):
        r, g, b, a = cmap(norm(i))
        ir = to_int(r, r_bits)
        ig = to_int(g, g_bits)
        ib = to_int(b, b_bits)
        ia = to_int(a, a_bits)
        #print(f"    {{ {r}, {g}, {b}, {a} }}, -> ", end="")
        print(f"  {{ {ir}, {ig}, {ib}, {ia} }},")


def main():
    print(f"#define PALETTE_SIZE {NUM_STEPS}")
    print()
    print("typedef struct")
    print("{")
    print("    uint8_t r, g, b, a;")
    print("} rgba;")
    print()

    for name in cmaps:
        cmap = plt.get_cmap(name)
        print(f"rgba palette_{name}[] =")
        print("{")
        cmap_to_struct(cmap, NUM_STEPS, R_BITS, G_BITS, B_BITS, A_BITS)
        print("};")
        print()

    print("rgba * palettes[] =")
    print("{")
    for name in cmaps:
        print(f"    palette_{name},")
    print("};")

if __name__ == '__main__':
    main()