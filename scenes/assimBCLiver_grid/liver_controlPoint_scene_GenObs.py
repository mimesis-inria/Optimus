import Sofa
import math
import os
import sys
import csv
import yaml

__file = __file__.replace('\\', '/') # windows


def createScene(rootNode):
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

    if len(commandLineArguments) > 1:
        configFileName = commandLineArguments[1]
    else:
        print 'ERROR: Must supply a yaml config file as an argument!'
        return
    print "Command line arguments for python : " + str(commandLineArguments)

    with open(configFileName, 'r') as stream:
        try:
            options = yaml.load(stream)

        except yaml.YAMLError as exc:
            print(exc)
            return

    liver_controlPoint_GenObs(rootNode, options)
    return 0


class liver_controlPoint_GenObs (Sofa.PythonScriptController):

    def __init__(self, rootNode, options):
        self.options = options
        self.vdamping = 5.0

        rootNode.findData('dt').value = options['scene_parameters']['general_parameters']['delta_time']
        rootNode.findData('gravity').value = options['scene_parameters']['general_parameters']['gravity']
        self.createGraph(rootNode)
        return None



    def createGraph(self, rootNode):
        nu=0.45
        E=5000

        # rootNode
        rootNode.createObject('VisualStyle', displayFlags='showVisualModels hideBehaviorModels hideCollisionModels hideMappings showForceFields')

        # rootNode/externalImpact
        dotNode = rootNode.createChild('dotNode')
        self.impactPoint = dotNode.createObject('MechanicalObject', template='Vec3d', name='dot', showObject='true', position='0.143369 0.156781 0.395153')
        if self.options['scene_parameters']['obs_generating_parameters']['save_observations']:
            dotNode.createObject('BoxROI', name='dotBounds', box='0.14 0.15 0.37 0.18 0.17 0.4', doUpdate='0')
            dotNode.createObject('Monitor', name='toolMonitor', template='Vec3d', showPositions='1', indices='@dotBounds.indices', ExportPositions='1', fileName=self.options['scene_parameters']['impact_parameters']['observation_file_name'])
        self.index = 0
        	
        # rootNode/simuNode
        simuNode = rootNode.createChild('simuNode')
        self.simuNode = simuNode
        simuNode.createObject('EulerImplicitSolver', rayleighStiffness=self.options['scene_parameters']['general_parameters']['rayleigh_stiffness'], rayleighMass=self.options['scene_parameters']['general_parameters']['rayleigh_mass'])
        simuNode.createObject('SparsePARDISOSolver', name='LDLsolver', verbose='0', symmetric='1', exportDataToFolder='')

        # simuNode.createObject('MeshVTKLoader', name='loader', filename=self.options.model.volumeFileName)
        simuNode.createObject('MeshGmshLoader', name='loader', filename=self.options['scene_parameters']['system_parameters']['volume_file_name'])
        simuNode.createObject('MechanicalObject', src='@loader', name='Volume')

        if 'boundary_conditions_list' in self.options['scene_parameters']['general_parameters'].keys():
            for index in range(0, len(self.options['scene_parameters']['general_parameters']['boundary_conditions_list'])):
                bcElement = self.options['scene_parameters']['general_parameters']['boundary_conditions_list'][index]
                print bcElement
                simuNode.createObject('BoxROI', box=bcElement['boxes_coordinates'], name='boundBoxes'+str(index), drawBoxes='0', doUpdate='0')
                if bcElement['condition_type'] == 'fixed':
                    simuNode.createObject('FixedConstraint', indices='@boundBoxes'+str(index)+'.indices')
                elif bcElement['condition_type'] == 'elastic':
                    simuNode.createObject('RestShapeSpringsForceField', stiffness=bcElement['spring_stiffness_values'], angularStiffness="1", points='@boundBoxes'+str(index)+'.indices')
                else:
                    print 'Unknown type of boundary conditions'

        simuNode.createObject('TetrahedronSetTopologyContainer', name="Container", src="@loader", tags=" ")
        simuNode.createObject('TetrahedronSetTopologyModifier', name="Modifier")        
        simuNode.createObject('TetrahedronSetTopologyAlgorithms', name="TopoAlgo")
        simuNode.createObject('TetrahedronSetGeometryAlgorithms', name="GeomAlgo")
        if 'total_mass' in self.options['scene_parameters']['general_parameters'].keys():
            simuNode.createObject('UniformMass', totalMass=self.options['scene_parameters']['general_parameters']['total_mass'])
        if 'density' in self.options['scene_parameters']['general_parameters'].keys():
            simuNode.createObject('MeshMatrixMass', printMass='0', lumping='1', massDensity=self.options['scene_parameters']['general_parameters']['density'], name='mass')

        lamb=(E*nu)/((1+nu)*(1-2*nu))
        mu=E/(2+2*nu)
        materialParams='{} {}'.format(mu,lamb)
        #simuNode.createObject('TetrahedralTotalLagrangianForceField', name='FEM', materialName='StVenantKirchhoff', ParameterSet=materialParams)
        simuNode.createObject('TetrahedronFEMForceField', updateStiffness='1', name='FEM', listening='true', drawHeterogeneousTetra='1', method='large', youngModulus='5000', poissonRatio='0.45')

        if self.options['scene_parameters']['obs_generating_parameters']['save_observations']:
            simuNode.createObject('BoxROI', name='observationBox', box='-1 -1 -1 1 1 1', doUpdate='0')
            simuNode.createObject('Monitor', name='ObservationMonitor', indices='@observationBox.indices', fileName=self.options['scene_parameters']['system_parameters']['observation_file_name'], ExportPositions='1', ExportVelocities='0', ExportForces='0')

        # rootNode/simuNode/attached
        ##attachedNode = simuNode.createChild('Attached')
        ##self.attachedNode = attachedNode
        ##attachedNode.createObject('MechanicalObject', template='Vec3d', name='dofs', showObject='true', position=self.options.impact.position)
        ##attachedNode.createObject('RestShapeSpringsForceField', name='Springs', stiffness='25', angularStiffness='1', external_rest_shape='@../../dotNode/dot')
        ##attachedNode.createObject('BarycentricMapping')
        ##attachedNode.createObject('BoxROI', name='impactBounds', box='0.08 0.1 0.31 0.22 0.27 0.46', doUpdate='0')
        ##attachedNode.createObject('Monitor', name='toolMonitor', template='Vec3d', showPositions='1', indices='@impactBounds.indices', ExportPositions='1', fileName=self.options.impact.positionFileName)
        simuNode.createObject('BoxROI', name='impactBounds', box='0.14 0.15 0.37 0.18 0.17 0.4', doUpdate='0')
        simuNode.createObject('RestShapeSpringsForceField', name='Springs', stiffness='10000', angularStiffness='1', external_rest_shape='@../dotNode/dot', points='@impactBounds.indices')

        return 0



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


