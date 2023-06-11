import matplotlib.pyplot as plt
import math
import drawer

class Barco:
    def __init__(self, length):
        self.length = length
        pass

    def setMaterial(self, density):
        self.density = density
    
    def setSections(self, sections:list, height):
        """! Define each section width and height
        """
        self.sections = sections
        self.height = height
        self.AreaSec = []
        for i in range(len(sections) - 1):
            print(i)
            self.AreaSec.append(self.calculateSectionArea(sections[i], sections[i+1], self.height))
        print(f"Areas {self.AreaSec}")
        self.rSections = self.sections.copy()
        self.rSections.reverse()
        self.rAreaSec = self.AreaSec.copy()
        self.rAreaSec.reverse()

        self.calculateTotalVolume()
    

    def calculateSectionArea(self,w0, w1, h):
        return 0.5*w0*h + 0.5*w1*h
    
    
    def calculateSubSectionArea(self, w0, w1, prof):
        """! w0 is always bigger than w1
            @param prof distance from w1."""
        profRel = prof/self.height
        return self.calculateSectionArea(profRel*w0 + (1-profRel)*w1, w1, prof)

    def calculateTotalVolume(self):
        totalArea = sum(self.AreaSec)

        self.totalArea = sum(self.AreaSec)
        self.totalVolume = totalArea*len(self.AreaSec)*self.height
        pass

    def calculatePush(self, prof):
        print(f"---- Prof: {prof}")
        sectionsUnder = math.floor(prof/self.height)
        print(f"Sections under: {sectionsUnder}")
        volumen = 0
        # Add all the submerged parts

        for i in range(sectionsUnder):
            eVolumen = self.rAreaSec[i]
            print(f"Adding volumen for {i} : {eVolumen}")
            volumen += eVolumen
        restante = prof -  sectionsUnder*self.height
        print(f"Restante: {restante}")
        if sectionsUnder < len(self.AreaSec) and restante > 0:
            numSecs = len(self.AreaSec)
            sectionW0 = self.sections[numSecs - sectionsUnder - 1]
            sectionW1 = self.sections[numSecs - sectionsUnder]
            eVolumen = self.calculateSubSectionArea(sectionW0, sectionW1, restante)
            print(f"Adding last volume {eVolumen}")
            volumen += eVolumen

        volumen *= self.length
        print(f"Volumen sumergido: {volumen}")
        push = volumen
        print(f"Push: {push}")
        return push

    def getWeight(self):
        return self.totalVolume*self.density




# All in cm 
sections = [10,7,0]
h = 6
 
b = Barco(100)
b.setSections(sections, h)
b.setMaterial(0.015) # [g/cm^3]

# Simulate different deeps
profs = range((len(sections)-1)*h)
outs = []
for i in profs:
    outs.append(b.calculatePush(i)-b.getWeight())

d = drawer.drawer()
d.plotData(profs, outs)
dontEraseMe = d.drawBoat(sections, h)
d.show()


