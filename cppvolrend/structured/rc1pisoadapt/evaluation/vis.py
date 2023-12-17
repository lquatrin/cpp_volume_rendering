from matplotlib import pyplot as plt
import pandas as pd


def Visualize1D(d, xdim, ydim, family=None, Title=None):

    # We condense the data using d.groupby() in the x-dimension of the plot.
    # https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.groupby.html
    # Essentially, we compute statistics over all occurences
    # of StepSize = 0.1, StepSize = 0.2, and so on, and then plot those.
    # We can do this for a second dimension at the same time,
    # thereby obtaining a multi-dimensional index leading us to
    # visualize a family of curves.

    # Grouping
    Groups = [xdim] if not family else [xdim] + [family]
    GroupedAvg = d.groupby(Groups).mean()
    VisData = GroupedAvg if not family else GroupedAvg.unstack(level=1)
    YAxisLabel = "Average " + ydim

    # Vis as line plot with markers
    VisData.plot.line(y=ydim, marker='o')

    # Basic attributes of the plot
    plt.title(Title)
    plt.ylabel(YAxisLabel)
    plt.xlabel(xdim)

    # x-axis ticks
    plt.xticks(GroupedAvg.index.get_level_values(0).drop_duplicates(keep='first'))

    plt.show()


def VisualizeScatter(d, xdim, ydim, size=None, color=None, Title=None, ShowWindow=True):

    d.plot.scatter(xdim, ydim, s=size, c=color, colormap='tab20c')

    # Basic attributes of the plot
    plt.title(Title)

    if ShowWindow: plt.show()



# Read as Pandas DataFrame
#~ d = pd.read_csv('EvalSSIM_aneurism.csv', quotechar='"')
d = pd.read_csv('EvalSSIM_bonsai.csv', quotechar='"')
#~ d = pd.read_csv('EvalSSIM_synthetic.csv', quotechar='"')

# Visualize1D(d, "StepSizeSmall", "TimePerFrame (ms)")
Visualize1D(d, "StepSizeSmall", "TimePerFrame (ms)", "StepSizeLarge")
# Visualize1D(d, "StepSizeSmall", "TimePerFrame (ms)", "StepSizeRange")
# Visualize1D(d, "StepSizeRange", "TimePerFrame (ms)", "StepSizeSmall")
# Visualize1D(d, "StepSizeLarge", "TimePerFrame (ms)", "StepSizeSmall")

# Visualize1D(d, "StepSizeRange", "SSIM", "StepSizeSmall")

VisualizeScatter(d, "SSIM", "TimePerFrame (ms)", color="StepSizeSmall", ShowWindow=False)
VisualizeScatter(d, "SSIM", "TimePerFrame (ms)", color="StepSizeLarge", ShowWindow=False)
VisualizeScatter(d, "SSIM", "TimePerFrame (ms)", color="StepSizeRange", ShowWindow=True)

