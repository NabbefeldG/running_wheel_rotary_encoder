import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


if __name__ == "__main__":
    filePath = "data/PTD+PT_2023-02-24_14-36-40.txt"
    # filePath = "PT+PTD_2023-03-03_11-34-54.txt"

    # data = pd.read_csv(filePath, delimiter=";", header=None)
    data = pd.read_csv(filePath, delimiter=";")

    # Find and remove "Timeout"-rows
    timeout_rows = np.isnan(data['Wheel1_cw_count'])
    # convert to numerical ids for the drop function
    timeout_rows = np.where(timeout_rows)[0]
    data = data.drop(timeout_rows)
    print("")

    # TODO search for name and genotype of animals based on the filename

    time = data['Time in ms']
    cw1 = data['Wheel1_cw_count']
    ccw1 = data['Wheel1_ccw_count']
    cw2 = data['Wheel2_cw_count']
    ccw2 = data['Wheel2_ccw_count']


    # Convert from millisecond x-axis to hours
    new_time = [eval(i) for i in time]
    new_time = [(x / (1000 * 60 * 60)) for x in new_time]

    samplestoplottime = new_time
    wheel1 = (cw1 + ccw1)/2
    wheel2 = (cw2 + ccw2)/2
    samplestoplotcw = cw1
    samplestoplotcw2 = cw2

    #plot the first hour
    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.set_title('Wheel turns within the first hour', fontsize=16)
    ax1.set_xlabel('Time [h]', fontsize=16)
    ax1.set_ylabel('Wheel turns', fontsize=16)
    ax1.tick_params(axis='y')
    ax1.tick_params(labelsize=12)
    # TODO change lim depending on data
    ax1.set_ylim([0, 400])
    ax1.set_xlim(right=int(1))

    plt.plot(new_time, wheel2, color='k')
    plt.plot(new_time, wheel1, color='g')

    plt.legend(['Control', 'Knockout'])
    plt.show()
    # TODO include variable in name so the dont overwrite each other
    plt.savefig("Wheelturns1h.png", transparent=True)
    plt.close()


    # plot the first 24h
    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.set_title('Wheel turns within the first 24 hours', fontsize=16)
    ax1.set_xlabel('Time [h]', fontsize=16)
    ax1.set_ylabel('Wheel turns', fontsize=16)
    ax1.tick_params(axis='y')
    ax1.tick_params(labelsize=12)
    # TODO change lim dependent on data
    ax1.set_ylim([0, 30000])
    ax1.set_xlim(right=int(24))

    plt.plot(new_time, wheel2, color='k')
    plt.plot(new_time, wheel1, color='g')

    # TODO make this depend on start time in file title
    plt.axvspan(3.5, 15.5, facecolor='grey', alpha=0.5)

    plt.legend(['Control', 'Knockout'])
    plt.show()

    plt.savefig("Wheelturnshabituation.png", transparent=True)
    plt.close()


    # bar graph diffpos/difftime
    new_time = np.array(new_time)
    events1 = np.diff(wheel1)
    events2 = np.diff(wheel2)
    relative_time = np.diff(new_time)

    plt.plot(new_time[1:], events1)
    plt.plot(new_time[1:], events2)

    eventsprotime = events1 / relative_time
    numTimeBins = int(np.ceil(new_time[-1]))

    binnedSpeed = np.zeros([2, numTimeBins])
    # eventsprotime[new_time[1:] < 1]

    for i in range(numTimeBins):
        # for j in range(numTimeBins):

        np.mean(eventsprotime[new_time[1:] < 1])
        binnedSpeed[0, i] = np.mean(eventsprotime[new_time[1:] < 1])
    #

    # find when they did first real run

    # compare active and inactive time: needs binned activity
    activeTime = wheel1[(new_time > 3.5) & (new_time < 15.5)]
    inactiveTime = wheel1[(new_time > 15.5) & (new_time < 27.5)]

    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.set_title('Wheel turns', fontsize=16)
    ax1.set_ylabel('Wheel turns', fontsize=16)
    ax1.tick_params(axis='y')
    ax1.tick_params(labelsize=12)
    both = [wheel2, wheel1]
    plt.boxplot(both)
    plt.xticks([1, 2], ['Control', 'Knockout'])
    plt.show()
    plt.savefig("Turns-boxplot_w1.png", transparent=True)
    plt.close()
#
