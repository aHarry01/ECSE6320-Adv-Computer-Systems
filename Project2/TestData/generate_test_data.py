width = 1500
height = 1500
outfile = f"M{width}x{height}.txt"

if __name__ == "__main__":
    
    f = open(outfile, "w")
    f.write(f"{width} {height}\n")
    for r in range(height):
        row = ""
        for w in range(width):
            row += f"{r*height + w} "
        f.write(row[:-1] + "\n")

    f.close()
