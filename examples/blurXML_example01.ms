/*!
	\file		blurXML_example01.ms
	
	\remarks	Usage exmaple for blurXML library
	
	\author		Diego Garcia
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		02/26/06
	
	\bug		3

	\version	
		- 1.0 - Created - DGH 02/26/06
*/


tabCount = 0

fn recurseNodes xmlNode toStr:undefined  =
(
	if (toStr == undefined) then
		toStr = listener
		
	-- we get the property names, nodes that have only one child which is text type
	local propNames = blurXML.getPropertyNames xmlNode
	local propCount = xmlNode.propCount

	-- we get the attribute names
	local attrNames = blurXML.getAttributenames xmlNode

	-- or access the attribute list this way
	local attList = xmlNode.attributes
	local attCount = xmlNode.attrCount

	-- we get the xmlNode names, xmlNode children that are NOT properties
	local nodeNames = blurXML.getNodeNames xmlNode
	local nodeCount = xmlNode.nodeCount
	local nodes = blurXML.getNodes xmlNode
	
	-- note that we still could access all the children this way
	local children = xmlNode.children
	local childrenCount = xmlNode.count
	childrenCount = xmlNode.children.count

	-- print the current xml xmlNode
	tabStr = ""
	for i = 1 to tabCount do
		tabStr += "\t"
		
	format "%< % >\n" tabStr xmlNode.nodeName to:toStr
	
	for a in attrNames do
		format "%#%: %\n" tabStr a xmlNode[a].value to:toStr

	for p in propNames do
		format "%%: %\n" tabStr p xmlNode[p].value to:toStr

	tabCount +=1
	
	-- recurse all the subnodes
	for n in nodes do	
		recurseNodes n toStr:toStr

	format "\n" to:toStr

	tabCount -=1
)

(
	str = stringstream ""
	
	-- we load a xml document
	xmlDoc = blurXML.loadDocument "C:/temp/blurXML_example01.xml"
	
	-- we recurse all the nodes to print them into the stringstream str
	recurseNodes xmlDoc.rootNode toStr:str
	
	format "%\n" str
	
	xmlDoc = undefined
)