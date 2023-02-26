import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


if __name__ == "__main__":
    filePath = "Actual_Mouse_2023-02-13_19-48-03.txt"

    data = pd.read_csv(filePath, delimiter=";", header=None)

    print("")

    time = []
    cw = []
    ccw = []
    for r in range(len(data)):
        print(data.iloc[r])
        time.append(int(data.iloc[r, 0].split(':')[1]))
        cw.append(int(data.iloc[r, 1].split(':')[1]))
        ccw.append(int(data.iloc[r, 2].split(':')[1]))
    #
    print(time)
    print(cw)
    print(ccw)

    plt.plot(time, cw)
    plt.plot(time, ccw)
    plt.show()

    print('')
#
