/*!
	\file		blurXML_example02.ms
	
	\remarks	Usage example for blurXML library. Saving xml files
	
	\author		Diego Garcia
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		02/26/06
	
	\bug		3

	\version	
		- 1.0 - Created - DGH 02/26/06
*/

-- we create a new xml document
--xmlDoc = blurXML.NewDocument()
blurdev = python.import "blurdev"
xmldoc = blurdev.XML.XMLDocument()

-- rootnode is the very basic node at the root of the document we have to use it in order to add elements
--xmlRoot = xmlDoc.rootNode
--xmlRoot = xmldoc.addNode_( "BlurAnimation" )

-- here we add several nodes
xmlRoot = xmlDoc.addNode_  "root"
xmlObjs = xmlRoot.addNode_ "objects"

--we are going to export the objects in the scene
for o in ($geometry as array) do
(
	-- create a new node with the object name
	nodeXml = xmlObjs.addNode_ o.name
	
	-- attributes are attached to the node
	nodeXml.setAttribute_ "name" (o.name)
	nodeXml.setAttribute_ "vertCount" ((o.mesh.numverts) as string)
	nodeXml.setAttribute_ "faceCount" ((o.mesh.numfaces) as string)
	
	props = getpropnames o
	
	if (props != undefined) then
	(
		-- we query for the properties of the object and save them as xml properties
		for p in props do
		(
			local pValue = try (getproperty o p) catch (undefined)
			print (nodeXml as string +" : " + p as string)
			nodeXml.setProperty p (pValue as string)
		)
	)
)

-- finally we save the document to disk
--blurXML.saveDocument xmlDoc "C:/temp/blurXML_example02.xml"
xmldoc.save("C:/temp/blurXML_example02.xml")


