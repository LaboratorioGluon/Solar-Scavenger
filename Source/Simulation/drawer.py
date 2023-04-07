import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider, Button
import math

class drawer:
    def __init__(self):
        self.profValue = 0
        pass

    def getWatH(self, deep):
        sectionsUnder = math.floor(deep/self.height)
        restante = deep -  sectionsUnder*self.height
        Hs1 = np.linspace((len(self.sections)-1)*self.height, 0, len(self.sections))
        Hs = list(Hs1)
        Hs.reverse()
        secs = self.sections.copy()
        secs.reverse()
        return np.interp(deep, Hs, secs)

    def plotData(self, prof:list, push:list):
        self.profs = prof
        self.push = push
        self.plotFig = plt.figure()
        plt.plot(prof, push)
        self.point, = plt.plot([0],[0], marker='o')
        
    
    def show(self):
        plt.show()

    def drawBoat(self, sections, height):
        self.sections = sections
        self.height = height
        self.getWatH(10)
        figW = max(sections)
        figH = height*(len(sections))
        figBoat = plt.figure()
        ax = figBoat.add_subplot(111)
        figBoat.subplots_adjust(left=0.25, bottom=0.25)
        ax.set_xlim(-figW/2-1, figW/2+1)
        ax.set_ylim(-1, figH+1)
        self.labelPush = ax.text(0,figH-1,"Holi")
        self.deepLine, = ax.plot([0,0], [0,0], linewidth=3)

        axDeep = figBoat.add_axes([0.25, 0.1, 0.65, 0.03])
        freq_slider = Slider(
            ax=axDeep,
            label='Deep[cm]',
            valmin=0,
            valmax=(len(sections)-1)*height,
            valinit=self.profValue,
        )

        freq_slider.on_changed(self.update)

        rightOffset = 2
        # Print horizontal
        for i in range(len(sections)):
            h = height*(len(sections)-i-1)
            sec = [-sections[i]/2, sections[i]/2 ]
            ax.plot(sec,[h, h], color="black")

        for i in range(len(sections)-1):
            h0 = height*(len(sections)-i-1)
            h1 = height*(len(sections)-i-2)
            sec0 = [-sections[i]/2, sections[i]/2 ]
            sec1 = [-sections[i+1]/2, sections[i+1]/2 ]
            ax.plot([sec0[0], sec1[0]] ,[h0, h1], color="black")
            ax.plot([sec0[1], sec1[1]] ,[h0, h1], color="black")

        return freq_slider

    def update(self,newValue):
        self.point.set_xdata(newValue)
        self.profValue = newValue
        newPush = np.interp(newValue,self.profs, self.push)
        self.point.set_ydata(newPush)
        self.labelPush.set_text(newPush)
        w=self.getWatH(newValue)
        self.deepLine.set_data([-w/2,w/2], [newValue, newValue])
        self.plotFig.canvas.draw_idle()
        pass