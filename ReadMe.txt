**************************************************
FLVParser
Created By: Jeff Mattson jmattson@cmu.edu
**************************************************

FLVParser est un petit programme simple, sous windows, mais qui s'effectuer "en ligne de commande DOS"
Il decoupe bien les flv selon le minutage pr¨¦-rep¨¦r¨¦.

Installer ces 4 fichiers dans un r¨¦pertoire, par exemple
C:\FLVParser  ; Puis

1) Aller dans ce r¨¦pertoire (contenant FLVParse.exe )

2) Y mettre (provisoirement) une copie du fichier monfichier.flv ¨¤ decouper, et le renommer 1.flv

3) Ce r¨¦pertoire contient aussi le fichier de commande parse.bat
Si vous faites clickdroit sur ce fichier / modifier 
Vous voyez apparaitre le texte suivant
--------------------------
flvparse.exe 1.flv cut.txt
--------------------------
Cela signifie simplement que son ex¨¦cution coupera le fichier 1.flv ¨¤ la dur¨¦e inscrite dans le fichier cut.txt
Donc inutile de modifier ce petit fichier ; vous le refermez ; il vous suffit de modifier le fichier cut.txt

4) En effet ce r¨¦pertoire contient aussi le fichier cut.txt
clickdroit sur ce fichier / modifier
Vous voyez apparaitre un minutage quelconque, ¨¤ remplacer par celui que vous souhaitez
-------------------------
	01:01:00:00
-------------------------
(avec	heures:minutes:secondes:centi¨¨mes de secondes)
(dans notre exemple on demanderait une coupure en deux, au bout de 1 heure et 1 minute juste)
Le modifier selon la dur¨¦e choisie...
(Conseil : Si on a plusieurs coupures ¨¤ faire, au bout de 1, puis 2, puis 3 minutes par exemple, commencer par couper ¨¤ 3 minutes, ce sera plus facile pour les coupures suivantes, avec cet ordre inverse)
Quitter cut.txt en sauvegardant la modification (avec le minutage de la coupure voulue).

5) Il ne reste plus qu'¨¤ Double-cliquer sur parse.bat
Attendre un peu...
	C'est fait, vos 2 morceaux 1_0.flv et 1_1.flv sont dans votre r¨¦pertoire.
	Vous pouvez par exemple jeter 1_1.flv puis recommencer apr¨¨s avoir renomm¨¦ 1_0.flv en 1.flv ...

6) Evidemment, au fur et ¨¤ mesure, il faut jeter ou renommer et d¨¦placer les morceaux ainsi cr¨¦¨¦s pour ses besoins...

**************************************************

**************************************************
FLVParser
Created By: Jeff Mattson jmattson@cmu.edu
**************************************************

FLVParser is a simple little program under Windows, but who perform "in DOS command line"
He etched flv well as the pre-spotted timing.

Install these four files in a directory, for example
C: \ FLVParser; Then

1) Go to this directory (containing FLVParse.exe)

2) to Y (temporarily) a copy of the file to be cut monfichier.flv and rename 1.flv

3) This directory also contains the command file parse.bat
If you clickdroit this file / edit
You see the following appear
--------------------------
flvparse.exe 1.flv cut.txt
--------------------------
This simply means that scripts will cut 1.flv File length listed in the file cut.txt
So no need to change this little file, you close, you just change the file cut.txt

4) In fact, this directory also contains the file cut.txt
clickdroit this file / edit
You see a timing appear any, to replace the one you want
-------------------------
01:01:00:00
-------------------------
(With hours: minutes: seconds: hundredths of seconds)
(In our example we require a cut in two, after 1 hour and 1 minute only)
Change according to the selected time ...
(Tip: If you have several cuts to make, at 1, then 2, then 3 minutes for example, start by cutting 3 minutes, it will be easier for the following denominations, with the reverse order)
Exit cut.txt safeguarding modification (with the timing of the desired cut.)

5) It remains only Double-click parse.bat
Wait a little ...
Done your two pieces and 1_0.flv 1_1.flv are in your directory.
For example, you can throw 1_1.flv then again after renaming 1_0.flv in 1.flv ...

6) Of course, as and when, should be discarded or rename and move the pieces and created for their needs ...

**************************************************

========================================================================
    CONSOLE APPLICATION : flvparser Project Overview
========================================================================

AppWizard has created this flvparser application for you.

This file contains a summary of what you will find in each of the files that
make up your flvparser application.


flvparser.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

flvparser.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

flvparser.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named flvparser.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
