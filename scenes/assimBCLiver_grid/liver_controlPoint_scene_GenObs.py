import Sofa
import math
import os
import sys
import csv
sys.path.append(os.getcwd() + '/../../python_src/configuration')
import DAOptions

__file = __file__.replace('\\', '/') # windows


def createScene(rootNode):
    rootNode.createObject('RequiredPlugin', pluginName='SofaPython')   
    rootNode.createObject('RequiredPlugin', name='Optimus', pluginName='Optimus')
    rootNode.createObject('RequiredPlugin', name='Pardiso', pluginName='SofaPardisoSolver')
    rootNode.createObject('RequiredPlugin', name='IMAUX', pluginName='ImageMeshAux')
    # rootNode.createObject('RequiredPlugin', name='MJED', pluginName='SofaMJEDFEM')
    rootNode.createObject('RequiredPlugin', pluginName='Geomagic')
    
    try : 
        sys.argv[0]
    except :
        commandLineArguments = []
    else :
        commandLineArguments = sys.argv
    mycylGravity_GenObs = cylGravity_GenObs(rootNode, commandLineArguments)
    return 0;


class cylGravity_GenObs (Sofa.PythonScriptController):

    options = DAOptions.DAOptions()

    def __init__(self, rootNode, commandLineArguments) :         

        # load configuration from yaml file
        if len(commandLineArguments) > 1:
            self.configFileName = commandLineArguments[1]
        else:
            self.configFileName = "cyl_scene_config.yml"

        self.options.parseYaml(self.configFileName)
        self.vdamping = 5.0

        rootNode.findData('dt').value = self.options.model.dt
        rootNode.findData('gravity').value = self.options.model.gravity

        print "Command line arguments for python : "+str(commandLineArguments)
        self.createGraph(rootNode)

        return None;

    def createGraph(self,rootNode):
        nu=0.45
        E=5000
        
        rootNode.createObject('VisualStyle', displayFlags='showVisualModels hideBehaviorModels hideCollisionModels hideMappings showForceFields')

        # rootNode/externalImpact
        dotNode = rootNode.createChild('dotNode')
        self.impactPoint = dotNode.createObject('MechanicalObject', template='Vec3d', name='dot', showObject='true', position='0.143369 0.156781 0.395153')
        if self.options.observations.save:
            dotNode.createObject('BoxROI', name='dotBounds', box='0.14 0.15 0.37 0.18 0.17 0.4')
            dotNode.createObject('Monitor', name='toolMonitor', template='Vec3d', showPositions='1', indices='@dotBounds.indices', ExportPositions='1', fileName=self.options.impact.positionFileName)
        self.index = 0
        	
        # rootNode/simuNode
        simuNode = rootNode.createChild('simuNode')
        self.simuNode = simuNode
        simuNode.createObject('EulerImplicitSolver', firstOrder='false', vdamping=self.vdamping, rayleighStiffness=self.options.model.rayleighStiffness, rayleighMass=self.options.model.rayleighMass)
        simuNode.createObject('SparsePARDISOSolver', name='LDLsolver', verbose='0', symmetric='1', exportDataToFolder='')
        # simuNode.createObject('MeshVTKLoader', name='loader', filename=self.options.model.volumeFileName)
        simuNode.createObject('MeshGmshLoader', name='loader', filename=self.options.model.volumeFileName)
        simuNode.createObject('MechanicalObject', src='@loader', name='Volume')
        for index in range(0, len(self.options.model.bcList)):
            bcElement = self.options.model.bcList[index]
            simuNode.createObject('BoxROI', box=bcElement.boundBoxes, name='boundBoxes'+str(index), drawBoxes='0')
            if (bcElement.bcType == 'fixed'):
                simuNode.createObject('FixedConstraint', indices='@boundBoxes'+str(index)+'.indices')
            elif (bcElement.bcType == 'elastic'):
                simuNode.createObject('RestShapeSpringsForceField', stiffness=bcElement.boundaryStiffness, angularStiffness="1", points='@boundBoxes'+str(index)+'.indices')
            else:
                print 'Unknown type of boundary conditions'
        simuNode.createObject('TetrahedronSetTopologyContainer', name="Container", src="@loader", tags=" ")
        simuNode.createObject('TetrahedronSetTopologyModifier', name="Modifier")        
        simuNode.createObject('TetrahedronSetTopologyAlgorithms', name="TopoAlgo")
        simuNode.createObject('TetrahedronSetGeometryAlgorithms', name="GeomAlgo")
        simuNode.createObject('UniformMass', totalMass=self.options.model.totalMass)

        lamb=(E*nu)/((1+nu)*(1-2*nu))
        mu=E/(2+2*nu)
        materialParams='{} {}'.format(mu,lamb)
        #simuNode.createObject('TetrahedralTotalLagrangianForceField', name='FEM', materialName='StVenantKirchhoff', ParameterSet=materialParams)
        simuNode.createObject('TetrahedronFEMForceField', updateStiffness='1', name='FEM', listening='true', drawHeterogeneousTetra='1', method='large', youngModulus='5000', poissonRatio='0.45')

        if self.options.observations.save:
            simuNode.createObject('BoxROI', name='observationBox', box='-1 -1 -1 1 1 1')
            simuNode.createObject('Monitor', name='ObservationMonitor', indices='@observationBox.indices', fileName=self.options.observations.valueFileName, ExportPositions='1', ExportVelocities='0', ExportForces='0')

        # rootNode/simuNode/attached
        ##attachedNode = simuNode.createChild('Attached')
        ##self.attachedNode = attachedNode
        ##attachedNode.createObject('MechanicalObject', template='Vec3d', name='dofs', showObject='true', position=self.options.impact.position)
        ##attachedNode.createObject('RestShapeSpringsForceField', name='Springs', stiffness='25', angularStiffness='1', external_rest_shape='@../../dotNode/dot')
        ##attachedNode.createObject('BarycentricMapping')
        ##attachedNode.createObject('BoxROI', name='impactBounds', box='0.08 0.1 0.31 0.22 0.27 0.46')
        ##attachedNode.createObject('Monitor', name='toolMonitor', template='Vec3d', showPositions='1', indices='@impactBounds.indices', ExportPositions='1', fileName=self.options.impact.positionFileName)
        simuNode.createObject('BoxROI', name='impactBounds', box='0.14 0.15 0.37 0.18 0.17 0.4')
        simuNode.createObject('RestShapeSpringsForceField', name='Springs', stiffness='10000', angularStiffness='1', external_rest_shape='@../dotNode/dot', points='@impactBounds.indices')

        return 0;



    def onEndAnimationStep(self, deltaTime):

        if self.index < 10000:
            position = self.impactPoint.findData('position').value
            position[0][1] = position[0][1] - 0.00002
            self.impactPoint.findData('position').value = position
            self.index = self.index + 1

        return 0;

    def onMouseButtonLeft(self, mouseX,mouseY,isPressed):
        ## usage e.g.
        #if isPressed : 
        #    print "Control+Left mouse button pressed at position "+str(mouseX)+", "+str(mouseY)
        return 0;

    def onKeyReleased(self, c):
        ## usage e.g.
        #if c=="A" :
        #    print "You released a"
        return 0;

    def initGraph(self, node):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onKeyPressed(self, c):
        ## usage e.g.
        #if c=="A" :
        #    print "You pressed control+a"
        return 0;

    def onMouseWheel(self, mouseX,mouseY,wheelDelta):
        ## usage e.g.
        #if isPressed : 
        #    print "Control button pressed+mouse wheel turned at position "+str(mouseX)+", "+str(mouseY)+", wheel delta"+str(wheelDelta)
        return 0;

    def storeResetState(self):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def cleanup(self):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onGUIEvent(self, strControlID,valueName,strValue):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onLoaded(self, node):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def reset(self):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onMouseButtonMiddle(self, mouseX,mouseY,isPressed):
        ## usage e.g.
        #if isPressed : 
        #    print "Control+Middle mouse button pressed at position "+str(mouseX)+", "+str(mouseY)
        return 0;

    def bwdInitGraph(self, node):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onScriptEvent(self, senderNode, eventName,data):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;

    def onMouseButtonRight(self, mouseX,mouseY,isPressed):
        ## usage e.g.
        #if isPressed : 
        #    print "Control+Right mouse button pressed at position "+str(mouseX)+", "+str(mouseY)
        return 0;

    def onBeginAnimationStep(self, deltaTime):
        ## Please feel free to add an example for a simple usage in /home/ip/Work/sofa/MyPlugins/Optimus/scenes/assimStiffness//home/ip/Work/sofa/master/src/applications/plugins/SofaPython/scn2python.py
        return 0;


