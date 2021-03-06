#include "header\file.h"
#include "header\group.h"
#include "header\WAD3.h"
#include "header\settings.h"
#include "header\utils.h"
#include "header\vertex.h"
#include "header\RMF.h"
#include "header\LSE.h"

#include <iostream>
#include <string>
#include <vector>
#include <time.h> //time
#include <conio.h>

#define PI 3.14159265

using namespace std;

/*
	====== PREAMBLE ======
	
		If you're looking at this code, I can assume you are interested in how to
		read, process and create Goldsource map files and its elements.
		
		Map2Curve is my second C++ project and I was learning about a lot of things
		while I was developing it over a timespan of 10 months (so far).
		It is not as efficiently or elegantly written as it could be and in no way
		it complies to any coding standards. Be assured that I am very aware of that.
		In the end I wasn't aiming to create a modern program, but to create
		something that helps me finishing my mod project and maybe even is of help
		for the Half-Life modding community.
		
		Most functions are written more or less inefficiently but were left as there
		are because of various reasons (time, motivation, you get the idea).
	
		Now one might wonder, apart from a programming language, what do you
		need to know if you were to write a map generator?
		
		Here are a few important items:
	
		- Trigonometry (sin, cos, tan, Pi)
		- Geometry / Analytic geometry (Vertices, Lines, Planes)
		- Linear Algebra (Vectors, Dot product, Cross product)
		- Matrices (especially Rotation Matrices), Euler Angles
	
	====== RESOURCES ======
	
		- Euler Angles: https://www.geometrictools.com/Documentation/EulerAngles.pdf
		- WAD 3 format: http://www.j0e.io
		
	====== C++ Info ====== 
	
		Based on GNU C++ 11
		Created in DevC++ 5.11
	
	
	
		Best regards
		ToTac
		Oct 25th 2019
		
		totac@web.de
		http://www.gibshooter.com
*/


// command list // 0 = bool, 1 = int, 2 = float, 3 = string, 4 = tform
vector<string> slist    { "rad", "offset", "res", "type", "obj", "shift", "target", "append", "tri", "round", "height", "ramp", "p_cornerfix", "p_reverse", "ramptex", "p_split", "splinefile", "scale", "rot", "move", "scale_src", "rot_src", "bounds", "range_start", "range_end", "transit_tri", "transit_round", "gaps", "gaplen", "skipnull", "d_enable", "d_autoyaw", "d_autopitch", "d_pos", "nulltex", "spike_height", "d_separate", "d_autoname", "d_pos_rand", "d_rotz_rand", "d_movey_rand", "d_draw", "d_draw_rand", "d_skip", "heightmode", "p_scale", "p_expand", "p_evenout", "map", "c_enable", "texmode", "d_carve", "d_autoassign", "d_circlemode", "flatcircle", "d_scale_rand", "rmf" };
vector<int> slist_id; 
vector<int> slist_type  {  2,     2,        1,     1,      0,     1,       3,        0,        0,     0,       2,        0,      0,             0,           0,         0,         3,            4,       4,     4,      4,           4,         1,        2,             2,           0,             0,               0,      2,        0,          0,          0,           0,             2,       3,         2,              0,            0,            4,            4,             4,              1,        0,             1,        1,            4,         2,          0,           0,     0,          1,         1,        1,               1,               0,           4,             0};
vector<int> slist_min   { -131072,-131072,  0,     0,      0,     0,       0,        0,        0,     0,       -131072,  0,      0,             0,           0,         0,         0,            0,       0,     0,      0,           0,         0,        0,             0,           0,             0,               0,      0,        0,          0,          0,           0,             0,       0,         0,              0,            0,            0,            0,             0,              0,        0,             0,        0,            0,         -131072,    0,           0,     0,          0,         0,        0,               0,               0,           10,            0};
vector<int> slist_max   { 131072, 131072,   384,   3,      1,     5,       255,      1,        1,     1,       131072,   1,      1,             1,           1,         1,         255,          30,      30,    30,     30,          1,         2,        100,           100,         1,             1,               1,      131072,   1,          1,          1,           1,             1,       15,        131072,         1,            1,            30,           30,            30,             384,      1,             384,      5,            30,         131072,    1,           1,     1,          1,         1,        1,               1,               1,           10,            1};


// Global Objects
	vertex Zero;
	vector<WADFile> WADFiles;
	
	file *gFile = nullptr; // global pointer for current file
	
	ctable *cTable = nullptr; // final construction Table
	ctable *sTable = nullptr; // original imported settings storage
	ctable *mTable = nullptr;
	
	bool ValidDefaults = 1;
	ctable *dTable = nullptr; // Defaults construction Table
	
	group *mGroup = nullptr; 	// Imported map
	group *sGroup = nullptr; 	// Transformed map (1 for each curve object)
	group *bGroup = nullptr; 	// Generated curves
	
	group *mDetailGroup = nullptr;	// original detail objects
	group_set *sDetailSet = nullptr; // copied detail objects for each individual curve object
	group_set *DetailSet = nullptr; // final detail objects
	
	string def_nulltex = "SOLIDHINT";
	float def_spikesize = 4;
	
	bool G_LOG = 0;
	bool G_AUTOCLOSE = 0;
	bool G_DEV = 0;
	
	string ROOT = "";
	
	int ErrorCode = 1;

int main(int argc, char *argv[])
{
	srand (time(NULL)); // to generate more random numbers with rand()
	
	// Get the root directory from argv[0]
	string MAIN(argv[0]);
	ROOT = MAIN.substr(  0, MAIN.rfind( '\\', MAIN.length() )+1  );
	
	// initialize slist ID vector
	slist_id.resize(slist.size()); for(int i=0;i<slist.size();i++) slist_id[i]=i;
	
	if (argc >= 2)
	{
		vector<string> CleanFileList;
		// look for starting parameters at the beginning
		for (int i = 1; i < argc; i++)
		{
			string C_PARAM = argv[i];
			if (C_PARAM[0]=='-') // param is probably a command
			{
				C_PARAM = C_PARAM.substr(1);
				if 		(C_PARAM=="autoclose") 	G_AUTOCLOSE = 1;
				//else if (C_PARAM=="log") 		G_LOG 		= 1;
				else if (C_PARAM=="dev") 		G_DEV 		= 1;
			}
			else CleanFileList.push_back(argv[i]); // param is probably a filepath
		}
		
		// check for valid files
		cout << "++---------------------++\n";
		cout << "||   Map2Curve v0.7    ||\n";
		cout << "||      by ToTac       ||\n";
		cout << "||   Jan 16th, 2020    ||\n";
		cout << "++---------------------++\n\n";
		
		int vcount = 0; // valid files counter
		vector<file> Filelist;
		for (int i = 0; i<CleanFileList.size(); i++)
		{
			cout << " Checking File #" << vcount+1 << endl;
			cout << "+-----------------------------------------------------+" << endl;
			
			string cFile = CleanFileList[i];
			
			//check if filetype is valid (txt and map)
			int filetype = CheckFileType(cFile);
			
			if (filetype==1||filetype==2)
			{
				Filelist.push_back(cFile);
				Filelist[vcount].type = filetype;
				Filelist[vcount].fID = i;
				Filelist[vcount].GetInfo();
				
				if (!Filelist[vcount].valid_map)
				Filelist.pop_back();
				else
				vcount++;
				
				cout << "+-----------------------------------------------------+\n\n";
			}
			else {
				cout << "|    [ERROR] Can't handle this file type! Please use *.txt and *.map files only!" << endl;
				cout << "|            File: " << cFile << endl;
				cout << "|" << endl;
				cout << "+-----------------------------------------------------+" << endl << endl;
			}
		}
		
		// Preparations
		if (Filelist.size()>0)
		{
			cout << "     [INFO] Found " << Filelist.size() << " valid file(s)." << endl;
			
			cout << endl << "            Scanning WAD Folder and WADList.txt..." << endl;
			LoadWads();
			cout << "            " << WADFiles.size() << " WAD files loaded:" << endl;
			for(int j=0; j<WADFiles.size(); j++)
			cout << "            #" << j+1 << " " << WADFiles[j].FilePath << endl;
			
			cout << endl << "     [INFO] Loading Default Settings (DEFAULTS.txt) in Root Dir..." << endl;
			LoadDefaultSettings();
			if (ValidDefaults)	cout<< "            Successfully loaded!" << endl;
			else				cout<< "            Not found or empty. Using internal Defaults!" << endl;
		}
		
		int t_valids = 0;
		// Process Files
		for (int i = 0; i < Filelist.size(); i++)
		{
			bool dev = 0;
			file &cFile = Filelist[i];
			gFile = &Filelist[i];
			cout << endl << "  Processing File #"<<i+1 << endl;
			cout << "+-----------------------------------------------------+" << endl;
			if (cFile.valid_cfg&&!cFile.InternalMapSettings)
			{
				cout << "|  Loading external settings ("<< cFile.path_cfg <<")..." << endl;
				GetSettings(cFile.str_cfg, cFile.settings, slist);
			}
			
			cout << "|  Loading Map File ("<< cFile.path_map <<")..." << endl;
			cFile.LoadMap();
			cout << "|" << endl;
			
			if (mGroup->valid)
			{
				cout << "|  Creating Source Objects..." << endl;
				cFile.createGroupSource(); // copy map brushes for each curve object in case there are individual transformations
				
				cout << "|  Creating Construction Tables..." << endl;
				if ( (cFile.type==1 && cFile.valid_cfg) || (cFile.type==2 && !cFile.InternalMapSettings ) ) {
					cout << "|  (Using external Settings from TXT-File)" << endl;
					sTable = createTableS(mGroup->t_arcs, cFile.settings,0); // create raw settings table
				}
				else if (cFile.type==2 && cFile.InternalMapSettings) {
					cout << "|  (Using Map-internal Settings from info_curve and info_curve_export Entities)" << endl;
					mGroup->t_arcs = cFile.t_iarcs;
					sTable = createTableS(cFile.t_iarcs, cFile.settingsM,1);
				}
				else
				{
					cout << "|  (Using Default Settings)" << endl;
					sTable = createTableS(mGroup->t_arcs, cFile.settings,0);
				}
				
				if(dev||G_DEV)
				for (int g = 0; g<mGroup->t_arcs; g++){
					if (sTable!=nullptr) {
					cout << " sTable #" << g << endl;
					sTable[g].Print();
					getch();}}
				
				createTableC(); // create final construction tables
				
				if(dev||G_DEV)
				for (int g = 0; g<mGroup->t_arcs; g++) {
					if (sTable!=nullptr) {
					cout << " cTable #" << g << endl;
					cTable[g].Print();
					getch();}}
				
				cout << "|  Creating Detail Objects..." << endl;
				cFile.createDetailGroupSource();
				
				bool IsSetSTforms = 0, IsSetMap = 0, IsSetObj = 0, IsSetRMF = 0;
				for (int g = 0; g<mGroup->t_arcs; g++)
				{
					if (cTable[g].scale_src.IsSet || cTable[g].rot_src.IsSet) IsSetSTforms = 1;
					if (cTable[g].map>0) IsSetMap = 1;
					if (cTable[g].obj>0) IsSetObj = 1;
					if (cTable[g].rmf>0) IsSetRMF = 1;
				}
				
				if (IsSetSTforms)
				{
					cout << "|  Applying Source Transformations..." << endl;
					cFile.TransformSource();
					createTableC();
				}
				cout << "|" << endl;
				cFile.FixDetailPos(); // fix x position of detail objects after eventual source transformation
				
				int validGroups = mGroup->t_arcs;
				for (int g = 0; g<mGroup->t_arcs; g++)
				{
					group &Group = sGroup[g];
					
					cout << "|" << endl << "|  Generating Curve #"<<g+1<<"..." << endl;
					
					//check for paths
					if (  cTable[g].type==3 || cTable[g].type==2  || cTable[g].heightmode==2  ) {
						cout << "|    Loading Spline File..." << endl;
						cFile.LoadSpline(g);
						if (!Group.valid) { validGroups--; } //!cFile.PathList[g].valid
					}
					
					if (Group.valid)
					{
						def_nulltex = cTable[g].nulltex;
						for (int i=0;i<def_nulltex.length();i++) def_nulltex[i] = toupper(def_nulltex[i]);
						def_spikesize = cTable[g].spike_height;
						
						int t = cTable[g].type;
						int hm = cTable[g].heightmode;
						cout << "|    Type:       "; if (t==0) cout << "Pi Circle" << endl; else if (t==1) cout << "Grid Circle" << endl; else if (t==2) cout << "Simple Spline Extrusion" << endl; else if (t==3) cout << "Intersecting Spline Extrusion" << endl;
						cout << "|    Radius:     "<< cTable[g].rad << endl;
						cout << "|    Depth:      "<< sGroup[g].SizeY << endl;
						cout << "|    Sides:      "<< cTable[g].res << endl;
						cout << "|    Height:     "<< cTable[g].height << endl;
						cout << "|    Max Height: "<< cTable[g].height * cTable[g].res << endl;
						cout << "|    Heightmode: "; if (hm==0) cout << "Linear Slope" << endl; else if (hm==1) cout << "Smooth Slope" << endl; else if (hm==2) cout << "Spline" << endl; else if (hm==3) cout << "Random Jagged" << endl;
						cout << "|    Transform:  "<< " move " <<cTable[g].move << " rotate " << cTable[g].rot << " scale " <<cTable[g].scale << endl;
						cout << "|    Range:      Section " << floor(cTable[g].res*(cTable[g].range_start/100.0))+1 << " - " << floor(cTable[g].res*(cTable[g].range_end/100.0)) << " of total " << cTable[g].res << endl; //
						cout << "|" << endl;
						
						cout << "|    Creating Detail Objects..." << endl;
						cFile.createDetailGroup(g);
						
						cout << "|    Creating Construction Framework..." << endl;
						cFile.createFramework(g);
						
						cout << "|    Creating Curve Objects..." << endl;
						cFile.createGroupBrush(g);
						
						cout << "|    Building Curve Brushes..." << endl;
						cFile.buildArcs(g);
						
						cout << "|    Texturing..." << endl;
						cFile.texturize(g);
						
						cout << "|    Performing Cleanup..." << endl;
						cFile.WeldVertices(g);
						cFile.FixBorderliner(g);
						
						if (  ( cTable[g].tri>0 || cTable[g].ramp>0 || cTable[g].transit_tri>0 || cTable[g].round>0 || cTable[g].transit_round>0 ) && cTable[g].type!=2  ) {
						cout << "|    Triangulating..." << endl;
						cFile.Triangulate(g); }
						
						if ( cTable[g].texmode==1 && ( cTable[g].ramp==0||(cTable[g].ramp>0&&cTable[g].height==0) ) && (cTable[g].type==0||cTable[g].type==1) ) {
						cout << "|    Applying Texture Mapping..." << endl;
						bGroup[g].ShearVectors(); }
						
						if (  cTable[g].ramp>0 ) {
						cout << "|    Creating Ramp..." << endl;
						cFile.RampIt(g); }
						
						if (  cTable[g].scale.IsSet || cTable[g].move.IsSet || cTable[g].rot.IsSet  ) {
						cout << "|    Applying Final Transformations..." << endl;
						cFile.TransformFinal(g); }
						
						if (  cTable[g].round>0 || cTable[g].transit_round>0  ) {
						cout << "|    Rounding Vertices..." << endl;
						cFile.roundCoords(g); }
						
						if (  cTable[g].bound>0  ) {
						cout << "|    Creating Bounding Boxes..." << endl;
						cFile.createBounds(g); }
						
						cout << "|" << endl;
						
						if(cTable[g].map==0 && cTable[g].obj==0 && cTable[g].rmf==0)
						{
							cout << "|    [WARNING] No output format set! Using RMF file format..."<< endl;
						}
					}
					else
					{
						if (!Group.ValidMesh) {
							cout << "|    [ERROR] The source transformations caused the"<< endl;
							cout << "|            original mesh to become invalid! Aborting..." << endl;
							cout << "|            Transformations were:" << endl;
							cout << "|            - rot_src   " << cTable[g].rot_src << endl;
							cout << "|            - scale_src " << cTable[g].scale_src << endl;
							cout << "|            For more information about valid source Brushes" << endl;
							cout << "|            see the Online documentation." << endl;
							cout << "|" << endl;
						}
						if (!Group.ValidSpline) {
							cout << "|    [ERROR] Spline file invalid! Aborting..."<< endl;
						}
						validGroups--;
					}
				}

				// print all detail objects
				if(dev)
				for (int d = 0; d<DetailSet->t_groups; d++)
				{
					group &dGroup = DetailSet->Groups[d];
					cout << dGroup;
				}
				if(dev) getch();
				
				if (validGroups>0) {
				if (IsSetMap) {
				cout << "|  Exporting selected data to MAP file \"";
				cFile.ExportToMap();
				cout << "\"..." << endl; }
				
				if (IsSetRMF) {
				cout << "|  Exporting selected data to RMF file \"";
				cFile.ExportToRMF();
				cout << "\"..." << endl; }
				
				
				if (IsSetObj) {
				cout << "|  Exporting selected data to OBJ file(s)..." << endl;
				cFile.ExportToObj(); }
				}
				
				cout << "|" << endl;
				cout <<	"+-----------------------------------------------------+" << endl << endl;
				
				t_valids = validGroups;
			}
			else {
				cout << "|  [ERROR] Map File doesn't seem to contain valid"<< endl;
				cout << "|          Brushes for Curve-Generation! Aborting..." << endl;
				cout << "|" << endl;
				cout << "|          For more information about valid source Brushes" << endl;
				cout << "|          see the Online documentation." << endl;
				cout << "|" << endl;
				cout <<	"+-----------------------------------------------------+" << endl << endl;
			}
			if (mGroup->valid&&t_valids>0) ErrorCode = 0;
			if (mGroup!=nullptr) {delete mGroup;   mGroup = nullptr; }
			if (bGroup!=nullptr) {delete[] bGroup; bGroup = nullptr; }
			if (sGroup!=nullptr) {delete[] sGroup; sGroup = nullptr; }
			if (cTable!=nullptr) {delete[] cTable; cTable = nullptr; }
			if (sTable!=nullptr) {delete[] sTable; sTable = nullptr; }
			if (mTable!=nullptr) {delete[] mTable; mTable = nullptr; }
			
			if (mDetailGroup!=nullptr) {delete[] mDetailGroup; mDetailGroup = nullptr; }
			if (sDetailSet!=nullptr) {delete[] sDetailSet; sDetailSet = nullptr; }
			if (DetailSet!=nullptr) {delete[] DetailSet; DetailSet = nullptr; }
		}
		
		if (dTable!=nullptr) {delete[] dTable; dTable = nullptr; }
		if(G_DEV) cout << " ############## ErrorCode " << ErrorCode << "  ############## " <<endl;
		cout << "\n\nPRESS ANY BUTTON TO EXIT..." << endl;
		if (!G_AUTOCLOSE) getch();
		//if (G_LOG) WriteTextToFile("LOG.txt", cout);
		
		return ErrorCode;
	}
	else // argc == 1
	{
		cout << "This program works by feeding it files.\n";
		cout << "Valid input files are Goldsource-Map (*.map) and Map2Curve Preset-Files (*.txt).";
		cout << "\n\nPRESS ANY BUTTON TO EXIT..." << endl;
		
		getch();
		return 0;
	}
	
	return 0;
	system("PAUSE");
}







