// store notes here
// https://github.com/CNMAT/OSC arduino osc, good stuff to get that thing going
//http://www.jamesalliban.com/#jahome good site for learning the industry
//http://jpbellona.com/kinect/ cool notes
// make our own web server https://msdn.microsoft.com/en-us/library/windows/desktop/aa364640(v=vs.85).aspx
//http://kepler.nasa.gov/multimedia/photos/imagesbykepler/?ImageID=14
//good pi notes http://cariadinteractive.com/wp-content/uploads/2015/01/Somatopia-Guide.pdf for of
// cool graphics http://nogoodexcuse.hubris.xyz/tag/opacity/
//http://zmc.space/2016/kinect-v2-introduction/ good tutorial
//https://forum.openframeworks.cc/t/interactive-mediawall-at-ipgs-nyc-headquarters/22012
//get cams from all over https://github.com/bakercp/ofxIpVideoGrabber
// 3d on the web https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API
// https://github.com/processing, another platform
// for javascript https://github.com/processing/p5.js (make my json go here too) http://p5js.org/
//http://www.mikewesthad.com/teaching_introduction_to_graphics.html
// license/rights http://nssdc.gsfc.nasa.gov/imgcat/html/group_page/ER.html, http://www.shadedrelief.com/natural3/pages/textures.html
//https://github.com/petewerner/generative
//https://github.com/ResEnv/SensorTape
//http://hubblesite.org/gallery/album/star/
//https://twitter.com/ofxaddons
//https://github.com/nickgillian/grt
// UN treaty info http://indianlaw.org/content/treaty-rights-and-un-declaration-rights-indigenous-peoples
//https://github.com/google/liquidfun
//http://www.kstrom.net/isk/maps/mn/treaties.html
// follow this https://github.com/openframeworks/openFrameworks/wiki/oF-code-style, but let them w/o bugs cast the first flames
//https://docs.google.com/presentation/d/1fEAb4-lSyqxlVGNPog3G1LZ7UgtvxfRAwR0dwd19G4g/edit#slide=id.g386b90fa9_01
//http://openframeworks.cc/ofBook/chapters/c++11.html\
//https://www.govtrack.us/congress/bills/110/hjres3/text/ih
//http://openframeworks.cc/tutorials/graphics/generativemesh.html
//http://hdl.handle.net/2027/pur1.32754077980880?urlappend=%3Bseq=1
//https://github.com/Kinect/Docs
//The radiocarbon date from the Sandy Lake Dam site provides solid evidence that Archaic inhabitants of northeastern Minnesota were actively involved in the early part of the Old Copper complex, well over six thousand years ago.  The Sandy Lake Dam site has integrity, with intact archaeological deposits containing cultural features and a broad artifact assemblage.  The site has the potential to provide data on a poorly understood time period of prehistory in the state for a variety of research issues such as subsistence, settlement and seasonality patterns, copper technology and use, lithic technology and raw material use, site function and activity areas, and trade and relationships to other cultural groups and regions.  It is this research potential, including providing an absolute date for the Old Copper complex, that demonstrates the importance of the Sandy Lake Dam site in Minnesota archaeology.
//https://www.academia.edu/11379142/Research_Note_Radiocarbon_Date_for_the_Old_Copper_Component_at_Site_21AK11
// good source, touches on the migration http://www.sjsu.edu/faculty/watkins/lakota.htm
// http://www.native-languages.org/maps.htm
// Old Copper, 3000-1000 BC (), https://www.jstor.org/stable/25669519?seq=19#page_scan_tab_contents
//http://www.wikiwand.com/en/Native_Americans_in_the_United_States
// 3000 BC, as early as 5560 BC, Copper in MN, The Copper Age (3500 – 2300 BC) of Europe
//http://www.slideshare.net/gherm6/minnesota-early-history
// start here https://en.wikipedia.org/wiki/Bradbury_Brook, oldest know spot in MN
// http://www.jstor.org/stable/20707848?seq=1#fndtn-page_thumbnails_tab_contents, study auther, at some point go meet with him
// ok? https://www.youtube.com/watch?v=FnJgQyctZRE, maybe better https://www.youtube.com/watch?v=gTQqWlfSi34
/* http://www.towahkon.org/Dakotahistory.html, ties to Bradbury brook? According to one Dakota creation story, a creation story that "figures prominently in Lakota/Dakota creation stories", the sacred lake Mde Wakan (Mille Lacs Lake) is where the Dakota emerged as human beings into this world. The sacred lake is where the Lakota/Dakota people's primary (or first) Garden of Eden site is located*/
// http://www.towahkon.org/Dakotahistory.html After the Ojibwe were forced to leave their East Coast homelands they settled in the Great Lakes region. "Like other Indian groups, the Ojibwe were forced westward beginning in the 1640s when the [British weapons armed] League of the Iroquois began to attack other tribes in the Great Lakes region to monopolize the fur trade....
//http://www.amazon.com/Waziyatawin/e/B001K7SRRU
// http://www.leg.state.mn.us/docs/2009/mandated/090376.pdf - native partnering quotes?
/* iclone import notes
	//model.setup("C:\\Users\\mark\\Documents\\iclone\\heidi\\heidi.fbx");
	//model.setup("..\\..\\heidi.fbx"); // should be in bin\data but need to figure how to that in git bugbug 
	From2552Software::TheModel model4;
	model4.setup("C:\\Users\\mark\\Documents\\iclone\\test1\\t8_motion0.bvh"); // should be in bin\data but need to figure how to that in git bugbug 
	model.setAnimations(model4);
	model.setup("C:\\Users\\mark\\Documents\\iclone\\test1\\t7.obj"); // should be in bin\data but need to figure how to that in git bugbug 
									  //model2.setup("C:\\Users\\mark\\Documents\\iclone\\reggie\\reggie_Catwalk.fbx");

									  //AssbinImporter import;
	Assimp::Exporter exporter;
	size_t size = exporter.GetExportFormatCount();
	for (int i = 0; ; ++i) {
		const aiExportFormatDesc*pd = exporter.GetExportFormatDescription(i);
		if (pd == nullptr) {
			break;
		}
	}
	
	//https://commons.wikimedia.org/wiki/File:Vincent_van_Gogh_-_Self-Portrait_-_Google_Art_Project.jpg
	//aiReturn r = exporter.Export(model.getAssimpScene(), "collada", "righthererightnow");
	string er1 = exporter.GetErrorString();
	Assimp::Importer importer;
	const aiScene *s = importer.ReadFile("righthererightnow", 0); // mem owned by importer
	//aiCopyScene(s, &model.getAssimpScene());
	string er = importer.GetErrorString();

	//model.setPlay(aiString("AnimStack::motion0"));
	model.setPlay(aiString("Motion"));

	//model2.setPlay(aiString("AnimStack::Catwalk"));

*/