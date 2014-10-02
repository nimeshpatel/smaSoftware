#!/usr/bin/perl -I/application/lib -w
do 'sma.pl';
##########################################
# Perl Script for Flux Calib             #
# Version which uses Nimesh's Lookup     #
# Prioritize Quasar List !!              #
#                                        #
# Author: S. Takakuwa (C.f.A.) 2004March #
#                                        #
# Sep.8  2004 Add ALL quasars            #
# Sep 22 2004 Calc EL by Myself          #
#             Add H.A. Range             #
##########################################


###########################################################################
# Modifications on Aug 31, 2006 by Shelbi Hostler:
# -- I compared the List of Quasars to the Submillimeter Calibrator list
# on the website http://sma1.sma.hawaii.edu/callist.html.  I added to
# the source list the quasars that were listed on the table as having
# been observed by the SMA at some point in time, that were in the SMA
# catalog, but that were not on the list.  If you experience any errors 
# with the list of source quasars in this script, please let me know:
# shostler@cfa.harvard.edu.  Thanks!
###########################################################################
# 
###########################################################################
# Modifications on September 19, 2005 by Brooks:
# -- I checked all of the sources against hal9000's SMA catalog, and either 
# typos were corrected or the sources were flagged -1 (do not observe)
# -- I replaced the fixed 50 deg. Sun avoidance angle with a variable 
# $SUNANGLE that can be set by the observer.  (50 deg. was too inefficient)
# -- I changed the min. elev. angle limit $ELLLIMIT from 20 to 30 degrees.
###########################################################################



############################################################
# Specify HourAngle, and Calibrator Here in J2000 NAME !!! #
############################################################
$HAULIMIT=4.0;   # Upper Hour Angle Limit
#$HAULIMIT=0.0;   # Upper Hour Angle Limit
#$HALLIMIT=0.0;   # Lower Hour Angle Limit
$HALLIMIT=-4.0;  # Lower Hour Angle Limit

$calib="3c84";
#$calib="0927+390";
#$calib="2232+117";
#$calib="0423-013";
#$calib="0359+509";
#$calib="1751+096";
#$calib="bllac";
#$calib="3c279";
#$calib="0530+135";
#$calib="0854+201";
#$calib="1058+015";
#$calib="1924-292";
#$calib="3c454.3";
#$calib="3c111";
#$calib="3c273";

###########################################################################
#
#  NOTE::  Please do NOT edit the source priorities in this script!  
#    Instead, make a new copy for yourself to edit and run.
#
###########################################################################

#########################
# List of Quasars       #
# Need to be updated !! #
#########################
# Priority 1: will be observed as a first priority if it is available
# Priority 0: will be observed randomly
# Priority -1: will not be observed
$source[0]="0005+383"; $ra[0]=+1.48823; $dec[0]=+38.33754; $priority[0]=0;
$source[1]="0006-063"; $ra[1]=+1.55789; $dec[1]=-6.39315; $priority[1]=0;
$source[2]="0010+109"; $ra[2]=+2.62919; $dec[2]=+10.97486; $priority[2]=0;
$source[3]="0013+408"; $ra[3]=+3.37971; $dec[3]=+40.86032; $priority[3]=0;
$source[4]="0014+612"; $ra[4]=+3.70340; $dec[4]=+61.29551; $priority[4]=0;
$source[5]="0019+203"; $ra[5]=+4.90773; $dec[5]=+20.36266; $priority[5]=0;
$source[6]="0019+734"; $ra[6]=+4.94077; $dec[6]=+73.45834; $priority[6]=0;
$source[7]="0050-094"; $ra[7]=+12.67215; $dec[7]=-9.48478; $priority[7]=0;
$source[8]="0051-068"; $ra[8]=+12.78418; $dec[8]=-6.83387; $priority[8]=0;
$source[9]="0102+584"; $ra[9]=+15.69068; $dec[9]=+58.40309; $priority[9]=0;
$source[10]="0106-405"; $ra[10]=+16.68795; $dec[10]=-40.57221; $priority[10]=0;
$source[11]="0108+015"; $ra[11]=+17.16155; $dec[11]=+1.58342; $priority[11]=0;
$source[12]="0112+227"; $ra[12]=+18.02427; $dec[12]=+22.74411; $priority[12]=0;
$source[13]="0113+498"; $ra[13]=+18.36253; $dec[13]=+49.80668; $priority[13]=0;
$source[14]="0115-014"; $ra[14]=+18.82125; $dec[14]=-1.45127; $priority[14]=0;
$source[15]="0116-116"; $ra[15]=+19.05217; $dec[15]=-11.60429; $priority[15]=0;
$source[16]="0121+043"; $ra[16]=+20.48693; $dec[16]=+4.37354; $priority[16]=0;
$source[17]="0121+118"; $ra[17]=+20.42331; $dec[17]=+11.83067; $priority[17]=0;
$source[18]="0125-000"; $ra[18]=+21.37018; $dec[18]=-0.09888; $priority[18]=0;
$source[19]="0132-169"; $ra[19]=+23.18120; $dec[19]=-16.91348; $priority[19]=0;
$source[20]="0136+478"; $ra[20]=+24.24415; $dec[20]=+47.85808; $priority[20]=0;
$source[21]="0137-245"; $ra[21]=+24.40977; $dec[21]=-24.51497; $priority[21]=0;
$source[22]="0141-094"; $ra[22]=+25.35763; $dec[22]=-9.47880; $priority[22]=0;
$source[23]="0149+059"; $ra[23]=+27.34321; $dec[23]=+5.93155; $priority[23]=0;
$source[24]="0152+221"; $ra[24]=+28.07525; $dec[24]=+22.11881; $priority[24]=0;
$source[25]="0204+152"; $ra[25]=+31.21006; $dec[25]=+15.23640; $priority[25]=0;
$source[26]="0204-170"; $ra[26]=+31.24031; $dec[26]=-17.02218; $priority[26]=0;
$source[27]="0205+322"; $ra[27]=+31.27052; $dec[27]=+32.20836; $priority[27]=0;
$source[28]="0217+017"; $ra[28]=+34.45398; $dec[28]=+1.74714; $priority[28]=0;
$source[29]="0217+738"; $ra[29]=+34.37839; $dec[29]=+73.82573; $priority[29]=0;
$source[30]="0222-346"; $ra[30]=+35.73501; $dec[30]=-34.69131; $priority[30]=0;
$source[31]="0224+069"; $ra[31]=+36.11845; $dec[31]=+6.98982; $priority[31]=0;
$source[32]="0228+673"; $ra[32]=+37.20855; $dec[32]=+67.35084; $priority[32]=0;
$source[33]="0237+288"; $ra[33]=+39.46836; $dec[33]=+28.80250; $priority[33]=0;
$source[34]="0238+166"; $ra[34]=+39.66221; $dec[34]=+16.61647; $priority[34]=0;
$source[35]="0239+042"; $ra[35]=+39.96360; $dec[35]=+4.27261; $priority[35]=0;
$source[36]="0241-082"; $ra[36]=+40.26999; $dec[36]=-8.25576; $priority[36]=0;
$source[37]="0242+110"; $ra[37]=+40.62155; $dec[37]=+11.01687; $priority[37]=0;
$source[38]="0244+624"; $ra[38]=+41.24040; $dec[38]=+62.46848; $priority[38]=0;
$source[39]="0303+472"; $ra[39]=+45.89684; $dec[39]=+47.27119; $priority[39]=0;
$source[40]="0309+104"; $ra[40]=+47.26510; $dec[40]=+10.48787; $priority[40]=0;
$source[41]="0310+382"; $ra[41]=+47.70783; $dec[41]=+38.24829; $priority[41]=0;
$source[42]="0313+413"; $ra[42]=+48.25818; $dec[42]=+41.33366; $priority[42]=0;
$source[43]="0325+469"; $ra[43]=+51.3346; $dec[43]=+46.91851; $priority[43]=0;
$source[44]="0325+226"; $ra[44]=+51.40339; $dec[44]=+22.40012; $priority[44]=0;
$source[45]="0334-401"; $ra[45]=+53.55689; $dec[45]=-40.14039; $priority[45]=0;
$source[46]="0336+323"; $ra[46]=+54.12545; $dec[46]=+32.30815; $priority[46]=0;
$source[47]="0339-017"; $ra[47]=+54.87891; $dec[47]=-1.77661; $priority[47]=0;
$source[48]="0340-213"; $ra[48]=+55.14837; $dec[48]=-21.32533; $priority[48]=0;
$source[49]="0346+540"; $ra[49]=+56.64377; $dec[49]=+54.01642; $priority[49]=0;
$source[50]="0348-278"; $ra[50]=+57.15894; $dec[50]=-27.82043; $priority[50]=0;
$source[51]="0354+467"; $ra[51]=+58.62507; $dec[51]=+46.72188; $priority[51]=0;
$source[52]="0359+509"; $ra[52]=+59.87395; $dec[52]=+50.96393; $priority[52]=0;
$source[53]="0401+042"; $ra[53]=+60.33297; $dec[53]=+4.22623; $priority[53]=0;
$source[54]="0403-360"; $ra[54]=+60.97396; $dec[54]=-36.08386; $priority[54]=0;
$source[55]="0405-131"; $ra[55]=+61.39168; $dec[55]=-13.13714; $priority[55]=0;
$source[56]="0406-384"; $ra[56]=+61.74598; $dec[56]=-38.44112; $priority[56]=0;
$source[57]="0410+769"; $ra[57]=+62.69002; $dec[57]=+76.94592; $priority[57]=0;
$source[58]="0415+448"; $ra[58]=+63.98551; $dec[58]=+44.88046; $priority[58]=0;
$source[59]="0422+023"; $ra[59]=+65.71756; $dec[59]=+2.32415; $priority[59]=0;
$source[60]="0423+418"; $ra[60]=+65.98337; $dec[60]=+41.83409; $priority[60]=0;
$source[61]="0423-013"; $ra[61]=+65.81584; $dec[61]=-1.34252; $priority[61]=0;
$source[62]="0424-379"; $ra[62]=+66.17602; $dec[62]=-37.93911; $priority[62]=0;
$source[63]="0424+006"; $ra[63]=+66.19518; $dec[63]=+0.60176; $priority[63]=0;
$source[64]="0428-379"; $ra[64]=+67.16843; $dec[64]=-37.93877; $priority[64]=0;
$source[65]="0440-435"; $ra[65]=+70.07158; $dec[65]=-43.55239; $priority[65]=0;
$source[66]="0442-002"; $ra[66]=+70.66109; $dec[66]=-0.29539; $priority[66]=0;
$source[67]="0449+113"; $ra[67]=+72.28196; $dec[67]=+11.35794; $priority[67]=0;
$source[68]="0449+635"; $ra[68]=+72.34712; $dec[68]=+63.53595; $priority[68]=0;
$source[69]="0455-462"; $ra[69]=+73.96155; $dec[69]=-46.26630; $priority[69]=0;
$source[70]="0457-234"; $ra[70]=+74.26325; $dec[70]=-23.41445; $priority[70]=0;
$source[71]="0501-019"; $ra[71]=+75.30338; $dec[71]=-1.98729; $priority[71]=0;
$source[72]="0502+061"; $ra[72]=+75.56436; $dec[72]=+6.15208; $priority[72]=0;
$source[73]="0502+416"; $ra[73]=+75.65828; $dec[73]=+41.65537; $priority[73]=0;
$source[74]="0505+049"; $ra[74]=+76.34660; $dec[74]=+4.99520; $priority[74]=0;
$source[75]="0509+056"; $ra[75]=+77.35819; $dec[75]=+5.69315; $priority[75]=0;
$source[76]="0510+180"; $ra[76]=+77.50987; $dec[76]=+18.01155; $priority[76]=0;
#The next one is a B1950 name, because the J2000 name didn't work
$source[77]="0521-365"; $ra[77]=+80.74160; $dec[77]=-36.45857; $priority[77]=0;
$source[78]="0530+135"; $ra[78]=+82.73507; $dec[78]=+13.53199; $priority[78]=0;
$source[79]="0532+075"; $ra[79]=+83.16249; $dec[79]=+7.54537; $priority[79]=0;
$source[80]="0533+483"; $ra[80]=+83.31611; $dec[80]=+48.38134; $priority[80]=0;
$source[81]="0538-440"; $ra[81]=+84.70984; $dec[81]=-44.08582; $priority[81]=0;
$source[82]="0539+145"; $ra[82]=+84.92653; $dec[82]=+14.56266; $priority[82]=0;
$source[83]="0555+398"; $ra[83]=+88.87836; $dec[83]=+39.81366; $priority[83]=0;
$source[84]="0605+405"; $ra[84]=+91.46190; $dec[84]=+40.50225; $priority[84]=0;
$source[85]="0607-085"; $ra[85]=+91.99875; $dec[85]=-8.58055; $priority[85]=0;
$source[86]="0609-157"; $ra[86]=+92.42063; $dec[86]=-15.71130; $priority[86]=0;
$source[87]="0629-199"; $ra[87]=+97.34901; $dec[87]=-19.98881; $priority[87]=0;
$source[88]="0646+448"; $ra[88]=+101.63344; $dec[88]=+44.85461; $priority[88]=0;
$source[89]="0648-307"; $ra[89]=+102.05874; $dec[89]=-30.73879; $priority[89]=0;
$source[90]="0650-166"; $ra[90]=+102.60242; $dec[90]=-16.62770; $priority[90]=0;
$source[91]="0710+475"; $ra[91]=+107.69210; $dec[91]=+47.53643; $priority[91]=0;
$source[92]="0717+456"; $ra[92]=+109.46605; $dec[92]=+45.63424; $priority[92]=0;
$source[93]="0721+713"; $ra[93]=+110.47270; $dec[93]=+71.34343; $priority[93]=0;
$source[94]="0725-009"; $ra[94]=+111.461; $dec[94]=-0.91571; $priority[94]=0;
$source[95]="0730-116"; $ra[95]=+112.57963; $dec[95]=-11.68683; $priority[95]=0;
$source[96]="0733+503"; $ra[96]=+113.46884; $dec[96]=+50.36918; $priority[96]=0;
$source[97]="0738+177"; $ra[97]=+114.53081; $dec[97]=+17.70528; $priority[97]=0;
$source[98]="0739+016"; $ra[98]=+114.82514; $dec[98]=+1.61795; $priority[98]=0;
$source[99]="0741+312"; $ra[99]=+115.29460; $dec[99]=+31.20006; $priority[99]=0;
$source[100]="0748+240"; $ra[100]=+117.15045; $dec[100]=+24.00670; $priority[100]=0;
$source[101]="0750+125"; $ra[101]=+117.71686; $dec[101]=+12.51801; $priority[101]=0;
$source[102]="0753+538"; $ra[102]=+118.25577; $dec[102]=+53.88323; $priority[102]=0;
$source[103]="0757+099"; $ra[103]=+119.27768; $dec[103]=+9.94301; $priority[103]=0;
$source[104]="0808+498"; $ra[104]=+122.16527; $dec[104]=+49.84348; $priority[104]=0;
$source[105]="0808-078"; $ra[105]=+122.06473; $dec[105]=-7.85275; $priority[105]=0;
$source[106]="0811+017"; $ra[106]=+122.86128; $dec[106]=+1.78117; $priority[106]=0;
$source[107]="0818+423"; $ra[107]=+124.56667; $dec[107]=+42.37928; $priority[107]=0;
$source[108]="0823+223"; $ra[108]=+125.85316; $dec[108]=+22.38425; $priority[108]=0;
$source[109]="0824+392"; $ra[109]=+126.23118; $dec[109]=+39.27831; $priority[109]=0;
$source[110]="0824+558"; $ra[110]=+126.19682; $dec[110]=+55.87852; $priority[110]=0;
$source[111]="0825+031"; $ra[111]=+126.45974; $dec[111]=+3.15681; $priority[111]=0;
$source[112]="0830+241"; $ra[112]=+127.71703; $dec[112]=+24.18328; $priority[112]=0;
$source[113]="0831+044"; $ra[113]=+127.95365; $dec[113]=+4.49419; $priority[113]=0;
$source[114]="0836-202"; $ra[114]=+129.16340; $dec[114]=-20.28320; $priority[114]=0;
$source[115]="0841+708"; $ra[115]=+130.35152; $dec[115]=+70.89505; $priority[115]=0;
$source[116]="0854+201"; $ra[116]=+133.70365; $dec[116]=+20.10851; $priority[116]=0;
$source[117]="0902-142"; $ra[117]=+135.57013; $dec[117]=-14.25858; $priority[117]=0;
$source[118]="0903+468"; $ra[118]=+135.76663; $dec[118]=+46.85115; $priority[118]=0;
$source[119]="0914+027"; $ra[119]=+138.65797; $dec[119]=+2.76646; $priority[119]=0;
$source[120]="0920+446"; $ra[120]=+140.24358; $dec[120]=+44.69833; $priority[120]=0;
$source[121]="0921+622"; $ra[121]=+140.40096; $dec[121]=+62.26449; $priority[121]=0;
$source[122]="0927+390"; $ra[122]=+141.76256; $dec[122]=+39.03913; $priority[122]=0;
$source[123]="0937+501"; $ra[123]=+144.30137; $dec[123]=+50.14780; $priority[123]=0;
$source[124]="0948+406"; $ra[124]=+147.23058; $dec[124]=+40.66239; $priority[124]=0;
$source[125]="0956+252"; $ra[125]=+149.20781; $dec[125]=+25.25446; $priority[125]=0;
$source[126]="0957+553"; $ra[126]=+149.40910; $dec[126]=+55.38271; $priority[126]=0;
$source[127]="0958+474"; $ra[127]=+149.58197; $dec[127]=+47.41884; $priority[127]=0;
$source[128]="0958+655"; $ra[128]=+149.69685; $dec[128]=+65.56523; $priority[128]=0;
$source[129]="1014+230"; $ra[129]=+153.69610; $dec[129]=+23.02127; $priority[129]=0;
$source[130]="1037-295"; $ra[130]=+159.31700; $dec[130]=-29.56745; $priority[130]=0;
$source[131]="1041+061"; $ra[131]=+160.32151; $dec[131]=+6.17137; $priority[131]=0;
$source[132]="1043+241"; $ra[132]=+160.78765; $dec[132]=+24.14317; $priority[132]=0;
$source[133]="1044+809"; $ra[133]=+161.09610; $dec[133]=+80.91096; $priority[133]=0;
$source[134]="1048+717"; $ra[134]=+162.11508; $dec[134]=+71.72665; $priority[134]=0;
$source[135]="1048-191"; $ra[135]=+162.02759; $dec[135]=-19.15992; $priority[135]=0;
$source[136]="1051+213"; $ra[136]=+162.95329; $dec[136]=+21.33120; $priority[136]=0;
$source[137]="1058+015"; $ra[137]=+164.62335; $dec[137]=+1.56634; $priority[137]=0;
$source[138]="1058+812"; $ra[138]=+164.54806; $dec[138]=+81.24241; $priority[138]=0;
$source[139]="1103+302"; $ra[139]=+165.80542; $dec[139]=+30.24519; $priority[139]=0;
$source[140]="1104+382"; $ra[140]=+166.11381; $dec[140]=+38.20883; $priority[140]=0;
$source[141]="1118+125"; $ra[141]=+169.73875; $dec[141]=+12.57826; $priority[141]=0;
$source[142]="1127-189"; $ra[142]=+171.76830; $dec[142]=-18.95484; $priority[142]=0;
$source[143]="1130-148"; $ra[143]=+172.52939; $dec[143]=-14.82427; $priority[143]=0;
$source[144]="1146+399"; $ra[144]=+176.74291; $dec[144]=+39.97620; $priority[144]=0;
$source[145]="1147-382"; $ra[145]=+176.75571; $dec[145]=-38.20306; $priority[145]=0;
$source[146]="1153+809"; $ra[146]=+178.30208; $dec[146]=+80.97477; $priority[146]=0;
$source[147]="1153+495"; $ra[147]=+178.35195; $dec[147]=+49.51912; $priority[147]=0;
$source[148]="1159+292"; $ra[148]=+179.88264; $dec[148]=+29.24551; $priority[148]=0;
$source[149]="1203+480"; $ra[149]=+180.87440; $dec[149]=+48.05379; $priority[149]=0;
$source[150]="1209-271"; $ra[150]=+182.26019; $dec[150]=-24.10577; $priority[150]=0;
$source[151]="1215-175"; $ra[151]=+183.94480; $dec[151]=-17.52928; $priority[151]=0;
$source[152]="1222+042"; $ra[152]=+185.59396; $dec[152]=+4.22105; $priority[152]=0;
$source[153]="1224+213"; $ra[153]=+186.22691; $dec[153]=+21.37955; $priority[153]=0;
$source[154]="1239+075"; $ra[154]=+189.85245; $dec[154]=+7.50477; $priority[154]=0;
$source[155]="1246-257"; $ra[155]=+191.69501; $dec[155]=-25.79702; $priority[155]=0;
$source[156]="1254+116"; $ra[156]=+193.65940; $dec[156]=+11.68497; $priority[156]=0;
$source[157]="1258-223"; $ra[157]=+194.72699; $dec[157]=-22.32531; $priority[157]=0;
$source[158]="1305-105"; $ra[158]=+196.38756; $dec[158]=-10.5554; $priority[158]=0;
$source[159]="1309+119"; $ra[159]=+197.39138; $dec[159]=+11.90682; $priority[159]=0;
$source[160]="1310+323"; $ra[160]=+197.61943; $dec[160]=+32.34549; $priority[160]=0;
$source[161]="1316-336"; $ra[161]=+199.03328; $dec[161]=-33.64977; $priority[161]=0;
$source[162]="1327+221"; $ra[162]=+201.75359; $dec[162]=+22.18060; $priority[162]=0;
$source[163]="1329+319"; $ra[163]=+202.47027; $dec[163]=+31.90307; $priority[163]=0;
$source[164]="1337-129"; $ra[164]=+204.41576; $dec[164]=-12.95686; $priority[164]=0;
$source[165]="1357+193"; $ra[165]=+209.26849; $dec[165]=+19.31871; $priority[165]=0;
$source[166]="1357-154"; $ra[166]=+209.29685; $dec[166]=-15.458; $priority[166]=0;
$source[167]="1357+767"; $ra[167]=+209.48072; $dec[167]=+76.72251; $priority[167]=0;
$source[168]="1408-078"; $ra[168]=+212.23534; $dec[168]=-7.87407; $priority[168]=0;
$source[169]="1415+133"; $ra[169]=+213.99508; $dec[169]=+13.33992; $priority[169]=0;
$source[170]="1419+543"; $ra[170]=+214.94415; $dec[170]=+54.38744; $priority[170]=0;
$source[171]="1419+383"; $ra[171]=+214.94423; $dec[171]=+38.36347; $priority[171]=0;
$source[172]="1427-421"; $ra[172]=+216.98457; $dec[172]=-42.1054; $priority[172]=0;
$source[173]="1446+173"; $ra[173]=+221.64728; $dec[173]=+17.35210; $priority[173]=0;
$source[174]="1454-377"; $ra[174]=+223.61421; $dec[174]=-37.79254; $priority[174]=0;
$source[175]="1458+042"; $ra[175]=+224.74732; $dec[175]=+4.27050; $priority[175]=0;
$source[176]="1504+104"; $ra[176]=+226.10408; $dec[176]=+10.49422; $priority[176]=0;
$source[177]="1505+034"; $ra[177]=+226.27699; $dec[177]=+3.44189; $priority[177]=0;
$source[178]="1506+426"; $ra[178]=+226.72101; $dec[178]=+42.65640; $priority[178]=0;
$source[179]="1507-168"; $ra[179]=+226.76995; $dec[179]=-16.87507; $priority[179]=0;
$source[180]="1510-057"; $ra[180]=+227.72330; $dec[180]=-5.71873; $priority[180]=0;
$source[181]="1512-090"; $ra[181]=+228.21055; $dec[181]=-9.09995; $priority[181]=0;
$source[182]="1513-102"; $ra[182]=+228.43706; $dec[182]=-10.20007; $priority[182]=0;
$source[183]="1516+002"; $ra[183]=+229.16757; $dec[183]=+0.25053; $priority[183]=0;
$source[184]="1517-243"; $ra[184]=+229.42422; $dec[184]=-24.37208; $priority[184]=0;
$source[185]="1522-275"; $ra[185]=+230.65698; $dec[185]=-27.503; $priority[185]=0;
$source[186]="1540+147"; $ra[186]=+235.20622; $dec[186]=+14.79608; $priority[186]=0;
$source[187]="1549+506"; $ra[187]=+237.32279; $dec[187]=+50.63494; $priority[187]=0;
$source[188]="1549+026"; $ra[188]=+237.37265; $dec[188]=+2.61699; $priority[188]=0;
$source[189]="1550+054"; $ra[189]=+237.64695; $dec[189]=+5.45290; $priority[189]=0;
$source[190]="1557-000"; $ra[190]=+239.46431; $dec[190]=-0.30670; $priority[190]=0;
$source[191]="1604-446"; $ra[191]=+241.12927; $dec[191]=-44.6922; $priority[191]=0;
$source[192]="1608+104"; $ra[192]=+242.19251; $dec[192]=+10.48549; $priority[192]=0;
$source[193]="1613+342"; $ra[193]=+243.42110; $dec[193]=+34.21331; $priority[193]=0;
$source[194]="1625-254"; $ra[194]=+246.44538; $dec[194]=-25.46065; $priority[194]=0;
$source[195]="1626-298"; $ra[195]=+246.52509; $dec[195]=-29.85749; $priority[195]=0;
$source[196]="1632+825"; $ra[196]=+248.13321; $dec[196]=+82.53789; $priority[196]=0;
$source[197]="1635+381"; $ra[197]=+248.81455; $dec[197]=+38.13458; $priority[197]=0;
$source[198]="1637+472"; $ra[198]=+249.43805; $dec[198]=+47.29273; $priority[198]=0;
$source[199]="1638+573"; $ra[199]=+249.55607; $dec[199]=+57.33999; $priority[199]=0;
$source[200]="1640+397"; $ra[200]=+250.12347; $dec[200]=+39.77945; $priority[200]=0;
$source[201]="1642+689"; $ra[201]=+250.53270; $dec[201]=+68.94438; $priority[201]=0;
$source[202]="1653+397"; $ra[202]=+253.46757; $dec[202]=+39.76017; $priority[202]=0;
$source[203]="1658+476"; $ra[203]=+254.51158; $dec[203]=+47.63034; $priority[203]=0;
$source[204]="1658+076"; $ra[204]=+254.53755; $dec[204]=+7.69098; $priority[204]=0;
$source[205]="1700-261"; $ra[205]=+255.22147; $dec[205]=-26.18103; $priority[205]=0;
$source[206]="1707+018"; $ra[206]=+256.89340; $dec[206]=+1.81269; $priority[206]=0;
$source[207]="1716+686"; $ra[207]=+259.05807; $dec[207]=+68.61076; $priority[207]=0;
$source[208]="1719+177"; $ra[208]=+259.80437; $dec[208]=+17.75179; $priority[208]=0;
$source[209]="1727+455"; $ra[209]=+261.86521; $dec[209]=+45.51104; $priority[209]=0;
$source[210]="1728+044"; $ra[210]=+262.10397; $dec[210]=+4.45136; $priority[210]=0;
$source[211]="1734+389"; $ra[211]=+263.58575; $dec[211]=+38.96429; $priority[211]=0;
$source[212]="1737+063"; $ra[212]=+264.30721; $dec[212]=+6.35098; $priority[212]=0;
$source[213]="1739+499"; $ra[213]=+264.86413; $dec[213]=+49.91760; $priority[213]=0;
$source[214]="1740+521"; $ra[214]=+265.15408; $dec[214]=+52.19539; $priority[214]=0;
$source[215]="1743-038"; $ra[215]=+265.99523; $dec[215]=-3.83462; $priority[215]=0;
$source[216]="1744-312"; $ra[216]=+266.09826; $dec[216]=-31.27666; $priority[216]=0;
$source[217]="1751+096"; $ra[217]=+267.88675; $dec[217]=+9.65020; $priority[217]=1;
#  The following source name is B1950 because the J2000 didn't work.
$source[218]="1751+288"; $ra[218]=+268.42700; $dec[218]=+28.80136; $priority[218]=0;
$source[219]="1800+388"; $ra[219]=+270.10319; $dec[219]=+38.80853; $priority[219]=0;
$source[220]="1800+440"; $ra[220]=+270.38465; $dec[220]=+44.07275; $priority[220]=0;
$source[221]="1800+784"; $ra[221]=+270.19035; $dec[221]=+78.46778; $priority[221]=0;
$source[222]="1801+440"; $ra[222]=+270.38465; $dec[222]=+44.07275; $priority[222]=0;
$source[223]="1824+568"; $ra[223]=+276.02945; $dec[223]=+56.85041; $priority[223]=0;
$source[224]="1830+063"; $ra[224]=+277.52475; $dec[224]=+6.32109; $priority[224]=0;
$source[225]="1833-210"; $ra[225]=+278.41619; $dec[225]=-21.06127; $priority[225]=1;
$source[226]="1842+681"; $ra[226]=+280.64017; $dec[226]=+68.15701; $priority[226]=0;
$source[227]="1848+323"; $ra[227]=+282.09205; $dec[227]=+32.31737; $priority[227]=0;
$source[228]="1848+327"; $ra[228]=+282.14318; $dec[228]=+32.73335; $priority[228]=0;
$source[229]="1849+670"; $ra[229]=+282.31697; $dec[229]=+67.09491; $priority[229]=0;
$source[230]="1911-201"; $ra[230]=+287.79022; $dec[230]=-20.11531; $priority[230]=1;
$source[231]="1923-210"; $ra[231]=+290.88412; $dec[231]=-21.07593; $priority[231]=0;
$source[232]="1924+156"; $ra[232]=+291.16440; $dec[232]=+15.67887; $priority[232]=0;
$source[233]="1924-292"; $ra[233]=+291.21273; $dec[233]=-29.24170; $priority[233]=1;
$source[234]="1925+211"; $ra[234]=+291.49835; $dec[234]=+21.10727; $priority[234]=0;
$source[235]="1927+612"; $ra[235]=+291.87678; $dec[235]=+61.29241; $priority[235]=0;
$source[236]="1927+739"; $ra[236]=+291.95206; $dec[236]=+73.96710; $priority[236]=0;
$source[237]="1937-399"; $ra[237]=+294.31757; $dec[237]=-39.9671; $priority[237]=0;
$source[238]="1955+515"; $ra[238]=+298.92808; $dec[238]=+51.53015; $priority[238]=0;
$source[239]="1957-387"; $ra[239]=+299.49925; $dec[239]=-38.75177; $priority[239]=0;
$source[240]="2000-178"; $ra[240]=+300.23788; $dec[240]=-17.81602; $priority[240]=1;
$source[241]="2005+778"; $ra[241]=+301.37916; $dec[241]=+77.87868; $priority[241]=0;
$source[242]="2007+404"; $ra[242]=+301.93727; $dec[242]=+40.49683; $priority[242]=0;
$source[243]="2009+724"; $ra[243]=+302.46792; $dec[243]=+72.48871; $priority[243]=0;
$source[244]="2011-157"; $ra[244]=+302.81546; $dec[244]=-15.77785; $priority[244]=0;
$source[245]="2012+464"; $ra[245]=+303.02349; $dec[245]=+46.48216; $priority[245]=0;
$source[246]="2015+371"; $ra[246]=+303.86972; $dec[246]=+37.18320; $priority[246]=0;
$source[247]="2016+165"; $ra[247]=+304.05775; $dec[247]=+16.54282; $priority[247]=0;
$source[248]="2023+318"; $ra[248]=+305.82924; $dec[248]=+31.88397; $priority[248]=0;
$source[249]="2025+337"; $ra[249]=+306.29518; $dec[249]=+33.71673; $priority[249]=0;
$source[250]="2049+100"; $ra[250]=+312.44110; $dec[250]=+10.05399; $priority[250]=0;
$source[251]="2101+036"; $ra[251]=+315.41181; $dec[251]=+3.69203; $priority[251]=0;
$source[252]="2109+355"; $ra[252]=+317.38282; $dec[252]=+35.54933; $priority[252]=0;
$source[253]="2123+055"; $ra[253]=+320.93549; $dec[253]=+5.58947; $priority[253]=0;
$source[254]="2131-121"; $ra[254]=+322.89692; $dec[254]=-12.118; $priority[254]=0;
$source[255]="2131-021"; $ra[255]=+323.54296; $dec[255]=-1.88812; $priority[255]=0;
$source[256]="2134-018"; $ra[256]=+323.54296; $dec[256]=-1.88812; $priority[256]=0;
$source[257]="2136+006"; $ra[257]=+324.16078; $dec[257]=+0.69839; $priority[257]=0;
$source[258]="2139+143"; $ra[258]=+324.75545; $dec[258]=+14.39333; $priority[258]=0;
$source[259]="2147+094"; $ra[259]=+326.79235; $dec[259]=+9.49630; $priority[259]=0;
$source[260]="2148+069"; $ra[260]=+327.02275; $dec[260]=+6.96072; $priority[260]=0;
$source[261]="2152+175"; $ra[261]=+328.10341; $dec[261]=+17.57717; $priority[261]=0;
$source[262]="2158-150"; $ra[262]=+329.52617; $dec[262]=-15.01926; $priority[262]=1;
$source[263]="2203+317"; $ra[263]=+330.81240; $dec[263]=+31.76063; $priority[263]=0;
$source[264]="2203+174"; $ra[264]=+330.86206; $dec[264]=+17.43007; $priority[264]=0;
$source[265]="2206-185"; $ra[265]=+331.54340; $dec[265]=-18.5941; $priority[265]=0;
$source[266]="2213-254"; $ra[266]=+333.26041; $dec[266]=-25.49169; $priority[266]=0;
$source[267]="2217+243"; $ra[267]=+334.25343; $dec[267]=+24.36278; $priority[267]=0;
$source[268]="2218-035"; $ra[268]=+334.71683; $dec[268]=-3.59358; $priority[268]=0;
$source[269]="2229-085"; $ra[269]=+337.41702; $dec[269]=-8.54845; $priority[269]=1;
$source[270]="2232+117"; $ra[270]=+338.15170; $dec[270]=+11.73081; $priority[270]=1;
$source[271]="2236+284"; $ra[271]=+339.09363; $dec[271]=+28.48261; $priority[271]=0;
$source[272]="2246-121"; $ra[272]=+341.57597; $dec[272]=-12.11424; $priority[272]=0;
$source[273]="2248-325"; $ra[273]=+342.1612; $dec[273]=-32.59783; $priority[273]=0;
$source[274]="2258-279"; $ra[274]=+344.52485; $dec[274]=-27.97257; $priority[274]=0;
$source[275]="2301+374"; $ra[275]=+345.36557; $dec[275]=+37.44701; $priority[275]=0;
$source[276]="2311+344"; $ra[276]=+347.77220; $dec[276]=+34.41969; $priority[276]=0;
$source[277]="2320+052"; $ra[277]=+350.18690; $dec[277]=+5.23054; $priority[277]=0;
$source[278]="2323-032"; $ra[278]=+350.88314; $dec[278]=-3.28473; $priority[278]=0;
$source[279]="2327+096"; $ra[279]=+351.88992; $dec[279]=+9.6693; $priority[279]=0;
$source[280]="2331-159"; $ra[280]=+352.91105; $dec[280]=-15.94917; $priority[280]=0;
$source[281]="2333-237"; $ra[281]=+353.48016; $dec[281]=-23.72796; $priority[281]=0;
$source[282]="2334+076"; $ra[282]=+353.55345; $dec[282]=+7.60765; $priority[282]=0;
$source[283]="2348-165"; $ra[283]=+357.01087; $dec[283]=-16.52001; $priority[283]=0;
$source[284]="2358-103"; $ra[284]=+359.54534; $dec[284]=-10.33573; $priority[284]=0;
$source[285]="3c84"; $ra[285]=+49.95067; $dec[285]=+41.51170; $priority[285]=0;
$source[286]="3c111"; $ra[286]=+64.58883; $dec[286]=+38.02662; $priority[286]=0;
$source[287]="3c120"; $ra[287]=+68.29623; $dec[287]=+5.35434; $priority[287]=0;
$source[288]="3c147"; $ra[288]=+85.65058; $dec[288]=+49.85201; $priority[288]=0;
$source[289]="3c207"; $ra[289]=+130.19829; $dec[289]=+13.20654; $priority[289]=0;
$source[290]="3c273"; $ra[290]=+187.27792; $dec[290]=+2.05239; $priority[290]=0; 
$source[291]="3c274"; $ra[291]=+187.70593; $dec[291]=+12.39112; $priority[291]=0;
$source[292]="3c279"; $ra[292]=+194.04653; $dec[292]=-5.78931; $priority[292]=0;
$source[293]="3c286"; $ra[293]=+202.78453; $dec[293]=+30.50916; $priority[293]=0;
$source[294]="3c309.1"; $ra[294]=+224.78160; $dec[294]=+71.67219; $priority[294]=0;
$source[295]="3c345"; $ra[295]=+250.74504; $dec[295]=+39.81028; $priority[295]=0;
$source[296]="3c371"; $ra[296]=+271.71117; $dec[296]=+69.82447; $priority[296]=0;
$source[297]="3c380"; $ra[297]=+277.38243; $dec[297]=+48.74616; $priority[297]=0;
$source[298]="3c395"; $ra[298]=+285.73308; $dec[298]=+31.99492; $priority[298]=0;
$source[299]="3c418"; $ra[299]=+309.65431; $dec[299]=+51.32018; $priority[299]=0;
$source[300]="3c446"; $ra[300]=+336.44691; $dec[300]=-4.95039; $priority[300]=1;
$source[301]="3c454.3"; $ra[301]=+343.49062; $dec[301]=+16.14821; $priority[301]=1;
$source[302]="bllac"; $ra[302]=+330.68038; $dec[302]=+42.27777; $priority[302]=1;
$source[303]="nrao530"; $ra[303]=+263.26128; $dec[303]=-13.08043; $priority[303]=1;
#$source[2]="ngc315"; $ra[264]=+14.45368; $dec[264]=+30.35245; $priority[264]=0;
$source[304]="0057+303"; $ra[304]=+14.45368; $dec[304]=+30.35245; $priority[304]=0;



#-----------------------------------------------------#
#---- Below this line observers should not modify ----#
# Constants
$pi=atan2(1,1)*4.0;
$rad2deg=180.0/$pi;
$deg2rad=1./$rad2deg;
$LAT=19.82420526388;  # Lattitude of the SMA (Degree)
$LAT=$LAT*$deg2rad;

$scan=7;        # How many scans
$scancal=7;        # How many scans on the calibrator quasar
$inttime=30.0;   # correlator integration time in seconds
$nflux=20;       # Integration Time for Flux Calibrators
$ELULIMIT=85.00; # Upper Elevation Limit of the SMA, Be careful!!!!!
$ELLLIMIT=25.0;  # Lower Elevation Limit of the SMA
$SUNANGLE=25.0;  # Sun avoidance angle (degrees)

$nicname="SMA";

$num=@source; # number of listed quasars
for ($i=0;$i<$num;$i++) {
    $el[$i]=0.0;
    $ha[$i]=10.0;
    $ra[$i]=$ra[$i]*$deg2rad;
    $dec[$i]=$dec[$i]*$deg2rad;
}



##################
# Initialization #
##################
$mypid=$$;            # check process id
$myname = ${0};
command("project -r -i $mypid -f $myname");
print "The process ID of this $myname script is $mypid\n";
checkANT();         # check antennas to be used
command("radio");   # note: command is also subroutine.
command("stopChopping");
command("integrate -t $inttime");
print "----- initialization done -----\n";
print "$nicname starts flux observations!!\n";
print "Good Luck, $nicname!!\n";


#########################################
# Check the Elevation of the Calibrator #
#########################################
getLST();
calcEL();
checkcalEL();


#####################################
# Start with the flux calibration   #
# Uranus, or Callisto, or Ganymede  #
#####################################


print "Start with Uranus and/or Jovian Moons if it is available .....\n";

# Uranus
$sourceCoordinates=`lookup -s uranus`;
chomp($sourceCoordinates);
($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
   print "Uranus is available, and will be observed right now.\n";
   command("observe -s uranus");  
   command("sleep 5");
   command("tsys");
   command("integrate -s $nflux -t $inttime -w");
#   command("integrate -s 0 -t $inttime -w");
}
else {
    print "Uranus is not available now.\n";
}

# Neptune
$sourceCoordinates=`lookup -s neptune`;
chomp($sourceCoordinates);
($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
   print "Neptune is available, and will be observed right now.\n";
   command("observe -s neptune");  
   command("sleep 5");
   command("tsys");
   command("integrate -s $nflux -t $inttime -w");
#   command("integrate -s 0 -t $inttime -w");
}
else {
    print "Uranus is not available now.\n";
}

# Callisto
$sourceCoordinates=`lookup -s callisto`;
chomp($sourceCoordinates);
($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
   print "Callisto is available, and will be observed right now.\n";
   command("observe -s callisto");
   command("sleep 5");
   command("tsys");
   command("integrate -s $nflux -t $inttime -w");
}
else {
    print "Callisto is not available now.\n";
}

# Ganymede
$sourceCoordinates=`lookup -s ganymede`;
chomp($sourceCoordinates);
($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
    print "Ganymede is available, and will be observed right now.\n";
    command("observe -s ganymede");
    command("sleep 5");
    command("tsys");
    command("integrate -s $nflux -t $inttime -w");
}
else {
    print "Ganymede is not available now.\n";
}


########################
# Start Observing Loop #
########################
$nloop=0;
print "Starting main loop...................\n";
while(1){
    printPID();
    $nloop=$nloop+1;  print "Loop No.= $nloop\n";
    getLST();
    calcEL();
    checkcalEL();
    pickupQS();

    if ($calskip==1) {      # If the calibrator is too high EL ..... or too close to the Sun
        for ($i=0;$i<$num_of_tar;$i++) {
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) {
                    print "$target[$i] is available, and will be observed right now.\n";
                    $priority[$tarpp[$i]]=-1;
                    command("observe -s $target[$i]");
                    command("sleep 5");
                    command("tsys");
                    command("integrate -s $scan -t $inttime -w");
                }
                else {
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    elsif ($calskip==0)  {  # If the calibrator is available ....
        command("observe -s $calib");
        command("sleep 5");
        command("tsys");
        command("integrate -s $scancal -t $inttime -w");

        for ($i=0;$i<$num_of_tar;$i++) {
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) {
                    print "$target[$i] is available, and will be observed right now.\n";
                    $priority[$tarpp[$i]]=-1;
                    command("observe -s $target[$i]");
                    command("sleep 5");
                    command("tsys");
                    command("integrate -s $scan -t $inttime -w");
                }
                else {
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    else {                  # If the calibrator is too low EL, finish observations!!
        print "------- Final Calibration ------- \n";
        # Calibrator
        $sourceCoordinates=`lookup -s $calib`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if (18.0<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
            command("observe -s $calib");
            command("sleep 5");
            command("tsys");
            command("integrate -s $scan -t $inttime -w");
        }
        else {
            print "Calibrator $calib is NOT available now, skip ......\n";
        }

        print "Ending with Uranus and/or Jovian Moons if it is available .....\n";
        # Uranus
        $sourceCoordinates=`lookup -s uranus`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
            print "Uranus is available, and will be observed right now.\n";
            command("observe -s uranus");  
            command("sleep 5");
            command("tsys");
            command("integrate -s $nflux -t $inttime -w");
        }
        else {
            print "Uranus is not available now.\n";
        }

        # Callisto
        $sourceCoordinates=`lookup -s callisto`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
            print "Callisto is available, and will be observed right now.\n";
            command("observe -s callisto");
            command("sleep 5");
            command("tsys");
            command("integrate -s $nflux -t $inttime -w");
        }
        else {
            print "Callisto is not available now.\n";
        }

        # Ganymede
        $sourceCoordinates=`lookup -s ganymede`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
            print "Ganymede is available, and will be observed right now.\n";
            command("observe -s ganymede");
            command("sleep 5");
            command("tsys");
            command("integrate -s $nflux -t $inttime -w");
        }
        else {
            print "Ganymede is not available now.\n";
        }


        print "Flux Observation is done!! \n";
        exit(1);
    }
}



###############
# Subroutines #
###############
# --- interrupt handler for CTRL-C ---
sub finish {
   exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID {
  print "The process ID of this $myname script is $mypid\n";
}

# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT {
  print "Checking antenna status ... \n";
  for ($i = 1; $i <=8; $i++) {
     $exist = `value -a $i -v antenna_status`;
     chomp($exist);
     if ($exist) {
        print "Found antenna ",$i," in the array!\n";
        @sma = (@sma,$i);
     }
  }
  print "Antennas @sma are going to be used.\n";
}

# --- performs a shell task with delay and printing. ---
sub command {
   my ($a);
   print "@_\n";                        # print the command
   $a=system("@_");                     # execute the command
#   sleep 1;                             # sleep 1 sec
   return $a;
}

# --- get LST in hours ---
# usage: getLST();
# This subroutine updates the global variable $LST, and prints
# its value.
sub getLST {
  $LST= `value -a $sma[0] -v lst_hours`;
  chomp($LST);
  print "LST [hr]= $LST\n";
}


# --- Calculate Elevation for each quasars ---
# usage: calcEL();
sub calcEL {
    print "Looking up available quasars ..........\n";
    for ($i=0;$i<$num;$i++) {
        $ha[$i] = $LST*15.0*$deg2rad-$ra[$i] ;
        $sinel[$i] = sin($LAT)*sin($dec[$i])+cos($LAT)*cos($dec[$i])*cos($ha[$i]);
        $cosel[$i] = (1.0 - $sinel[$i]**2.0)**0.5;
        $el[$i] = atan2($sinel[$i],$cosel[$i])*$rad2deg ;
        $ha[$i] = $ha[$i]*$rad2deg/15.0 ;
        if ($ha[$i]>12.0) {$ha[$i]=$ha[$i]-24.0;}
        if ($ha[$i]<-12.0) {$ha[$i]=$ha[$i]+24.0;}

        # print "SINE(EL) = $sinel[$i] \n" ;
        # print "COSINE(EL) = $cosel[$i] \n" ;
        # print "$source[$i] at EL=$el[$i] (Degree) and HA=$ha[$i] (hour)\n"
    }
}

#
# --- Check if your calibrator is usable .... ----
sub checkcalEL {
    $calname_correct=0;
    for ($i=0;$i<$num;$i++) {
        if($source[$i] eq $calib){
            # $elcal=$el[$i];
            $calname_correct=1;
        }
    }
    if ($calname_correct==0) {
        print "Calibrator Name $calib is wrong!! \n";
        print "Maybe you used B1950 name ...\n";
        print "Use J2000 name instead of B1950 name.\n";
        print "Abort this script ....\n";
        exit(1);
    }
    $Coordcal=`lookup -s $calib`;
    if ($Coordcal eq '') {
        print "#######################################\n";
        print "###### ERROR ERROR ERROR ########\n";
        print "##### calibrator $calib not found. ######\n";
        print "#######################################\n";
        print "Abort this script ..... \n";
        exit(1);
    }
    chomp($Coordcal);
    ($azcal,$elcal,$suncal)=split(' ',$Coordcal);
    print "Calibrator $calib at EL=$elcal (Degree) and Az=$azcal (Degree) and SunDist=$suncal (Degree)\n";
    if ($elcal<$ELLLIMIT && $suncal>$SUNANGLE) {
        print "Calibrator $calib is at too low EL=$elcal (deg) ...\n";
        print "                \n";
        print "You may be at the end of the flux track. \n";
        print "Thank you very much for your hard work. \n";
        print "Will do final sequence \n";
        print "                \n";
        print "If you have not done anything yet, \n";
        print "Please wait for a while, or use a different calibrator. \n";
        print "And.... You may kill this script with the process ID $mypid \n";
        $calskip=-1;
#        exit(1);
    }
    elsif ($ELLLIMIT<=$elcal && $elcal<$ELULIMIT && $suncal>$SUNANGLE) {
        print "Calibrator $calib is in the observable EL range.\n";
        $calskip=0;
    }
    else {
        print "Calibrator $calib is at too high EL=$elcal (deg) ... \n";
        print "Or Calibrator $calib is too close to the Sun at $suncal (deg) ... \n";
        $calskip=1;
    }
}


#
# --- Pick Up Quasars to Observe ---
sub pickupQS {
# Priority 1: will be observed as a first priority if it is available
# Priority 0: will be observed randomly
# Priority -1: will not be observed
#

    $num_of_tar=0;

#############################
# Check if all the priority 
#############################
    $num_of_can=0;
    for ($i=0;$i<$num;$i++) {
        if ($source[$i] ne $calib && $priority[$i]>-1) {   
            $num_of_can++;
        }
    }
    print "Number of Unobserved Quasars except for calibrator is $num_of_can .\n";
    if ($num_of_can==0) {
        print("No More Quasar to be observed...., Abort .. \n");
        exit(1);
    }

###########################################################
# First, pick up candidates of the first priority quasars
###########################################################
    $num_of_can1=0;
    for ($i=0;$i<$num;$i++) {
        if ($el[$i]>$ELLLIMIT && $el[$i]<$ELULIMIT && $ha[$i]>$HALLIMIT && $ha[$i]<$HAULIMIT && $source[$i] ne $calib && $priority[$i]==1) {   
            $candidates1[$num_of_can1]=$source[$i];
            $canha1[$num_of_can1]=$ha[$i];
            $canel1[$num_of_can1]=$el[$i];
            $can_num1[$num_of_can1]=$i;
            $num_of_can1++;
        }
    }
    print "Number of Available Quasars except for calibrator at the highest priority is $num_of_can1 .\n";


# Switch depending on the number of Available Quasars at the highest priority
#
######################################################
# if there are more than 3 highest priority quasars
######################################################
    if ($num_of_can1>3) {
        $num_of_tar=3;
        $s1=int(rand($num_of_can1));
        $s2=int(rand($num_of_can1));
        while ($s2==$s1){$s2=int(rand($num_of_can1));}
        $s3=int(rand($num_of_can1));
        while ($s3==$s2 || $s3==$s1){$s3=int(rand($num_of_can1));}

        $target[0]=$candidates1[$s1];
        $tarha[0]=$canha1[$s1];
        $tarel[0]=$canel1[$s1];
        $tarpp[0]=$can_num1[$s1];
      #  $priority[$can_num1[$s1]]=-1;
        $target[1]=$candidates1[$s2];
        $tarha[1]=$canha1[$s2];
        $tarel[1]=$canel1[$s2];
        $tarpp[1]=$can_num1[$s2];
      #  $priority[$can_num1[$s2]]=-1;
        $target[2]=$candidates1[$s3];
        $tarha[2]=$canha1[$s3];
        $tarel[2]=$canel1[$s3];
        $tarpp[2]=$can_num1[$s3];
      #  $priority[$can_num1[$s3]]=-1;
    }
######################################################
# if there are exactly 3 highest priority quasars
######################################################
    elsif ($num_of_can1==3){
        $num_of_tar=3;
        $target[0]=$candidates1[0];
        $tarha[0]=$canha1[0];
        $tarel[0]=$canel1[0];
        $tarpp[0]=$can_num1[0];
      #  $priority[$can_num1[0]]=-1;
        $target[1]=$candidates1[1];
        $tarha[1]=$canha1[1];
        $tarel[1]=$canel1[1];
        $tarpp[1]=$can_num1[1];
      #  $priority[$can_num1[1]]=-1;
        $target[2]=$candidates1[2];
        $tarha[2]=$canha1[2];
        $tarel[2]=$canel1[2];
        $tarpp[2]=$can_num1[2];
      #  $priority[$can_num1[2]]=-1;
    }
######################################################
# if there are only 2 highest priority quasars
######################################################
    elsif ($num_of_can1==2){
        $target[0]=$candidates1[0];
        $tarha[0]=$canha1[0];
        $tarel[0]=$canel1[0];
        $tarpp[0]=$can_num1[0];
      #  $priority[$can_num1[0]]=-1;
        $target[1]=$candidates1[1];
        $tarha[1]=$canha1[1];
        $tarel[1]=$canel1[1];
        $tarpp[1]=$can_num1[1];
      #  $priority[$can_num1[1]]=-1;

        # Pick Up Another one from the second priority quasars
        $num_of_can2=0;
        for ($i=0;$i<$num;$i++) {
            if ($el[$i]>$ELLLIMIT && $el[$i]<$ELULIMIT && $ha[$i]>$HALLIMIT && $ha[$i]<$HAULIMIT && $source[$i] ne $calib && $priority[$i]==0) {   
                $candidates2[$num_of_can2]=$source[$i];
                $canha2[$num_of_can2]=$ha[$i];
                $canel2[$num_of_can2]=$el[$i];
                $can_num2[$num_of_can2]=$i;
                $num_of_can2++;
            }
        }
        if ($num_of_can2>0) {
            $num_of_tar=3;
            $s1=int(rand($num_of_can2));
            $target[2]=$candidates2[$s1];
            $tarha[2]=$canha2[$s1];
            $tarel[2]=$canel2[$s1];
            $tarpp[2]=$can_num2[$s1];
          #  $priority[$can_num2[$s1]]=-1;
        }
        else {
            $num_of_tar=2;
        }
    }
################################################
# if there is only 1 highest priority quasar
################################################
    elsif ($num_of_can1==1){
        $target[0]=$candidates1[0];
        $tarha[0]=$canha1[0];
        $tarel[0]=$canel1[0];
        $tarpp[0]=$can_num1[0];
      #  $priority[$can_num1[0]]=-1;

        # Pick Up Two more from the second priority quasars
        $num_of_can2=0;
        for ($i=0;$i<$num;$i++) {
            if ($el[$i]>$ELLLIMIT && $el[$i]<$ELULIMIT && $ha[$i]>$HALLIMIT && $ha[$i]<$HAULIMIT && $source[$i] ne $calib && $priority[$i]==0) {   
                $candidates2[$num_of_can2]=$source[$i];
                $canha2[$num_of_can2]=$ha[$i];
                $canel2[$num_of_can2]=$el[$i];
                $can_num2[$num_of_can2]=$i;
                $num_of_can2++;
            }
        }
        if ($num_of_can2>2) {
            $num_of_tar=3;
            $s1=int(rand($num_of_can2));
            $s2=int(rand($num_of_can2));
            while ($s2==$s1){$s2=int(rand($num_of_can2));}
            $target[1]=$candidates2[$s1];
            $tarha[1]=$canha2[$s1];
            $tarel[1]=$canel2[$s1];
            $tarpp[1]=$can_num2[$s1];
         #   $priority[$can_num2[$s1]]=-1;
            $target[2]=$candidates2[$s2];
            $tarha[2]=$canha2[$s2];
            $tarel[2]=$canel2[$s2];
            $tarpp[2]=$can_num2[$s2];
         #   $priority[$can_num2[$s2]]=-1;
        }
        elsif ($num_of_can2==2) {
            $num_of_tar=3;
            $target[1]=$candidates2[0];
            $tarha[1]=$canha2[0];
            $tarel[1]=$canel2[0];
            $tarpp[1]=$can_num2[0];
          #  $priority[$can_num2[0]]=-1;
            $target[2]=$candidates2[1];
            $tarha[2]=$canha2[1];
            $tarel[2]=$canel2[1];
            $tarpp[2]=$can_num2[1];
          #  $priority[$can_num2[1]]=-1;
        }
        elsif ($num_of_can2==1) {
            $num_of_tar=2;
            $target[1]=$candidates2[0];
            $tarha[1]=$canha2[0];
            $tarel[1]=$canel2[0];
            $tarpp[1]=$can_num2[0];
          #  $priority[$can_num2[0]]=-1;
        }
        else {
            $num_of_tar=1;
        }
    }
######################################################
# if there is no highest priority quasars
######################################################
    else {
        $num_of_can2=0;
        for ($i=0;$i<$num;$i++) {
            if ($el[$i]>$ELLLIMIT && $el[$i]<$ELULIMIT && $ha[$i]>$HALLIMIT && $ha[$i]<$HAULIMIT && $source[$i] ne $calib && $priority[$i]==0) {
                $candidates2[$num_of_can2]=$source[$i];
                $canha2[$num_of_can2]=$ha[$i];
                $canel2[$num_of_can2]=$el[$i];
                $can_num2[$num_of_can2]=$i;
                $num_of_can2++;
            }
        }

        if ($num_of_can2<4) {
            $num_of_tar=$num_of_can2;
            @target=@candidates2;
            @tarha=@canha2;
            @tarel=@canel2;
            for ($j=0;$j<$num_of_can2;$j++) {
                $tarpp[$j]=$can_num2[$j];
              #  $priority[$can_num2[$j]]=-1;
            }
        }
        else {
            $num_of_tar=3;
            $s1=int(rand($num_of_can2));
            $s2=int(rand($num_of_can2));
            while ($s2==$s1){$s2=int(rand($num_of_can2));}
            $s3=int(rand($num_of_can2));
            while ($s3==$s2 || $s3==$s1){$s3=int(rand($num_of_can2));}

            $target[0]=$candidates2[$s1];
            $tarha[0]=$canha2[$s1];
            $tarel[0]=$canel2[$s1];
            $tarpp[0]=$can_num2[$s1];
           # $priority[$can_num2[$s1]]=-1;
            $target[1]=$candidates2[$s2];
            $tarha[1]=$canha2[$s2];
            $tarel[1]=$canel2[$s2];
            $tarpp[1]=$can_num2[$s2];
           # $priority[$can_num2[$s2]]=-1;
            $target[2]=$candidates2[$s3];
            $tarha[2]=$canha2[$s3];
            $tarel[2]=$canel2[$s3];
            $tarpp[2]=$can_num2[$s3];
           # $priority[$can_num2[$s3]]=-1;
        }
    }

    print "Calibrator = $calib \n";
    print "-------------------------------------------------------------\n";
    print "If Calibrator is available\n";
    for ($i=0;$i<$num_of_tar;$i++) {
        print "$nicname will observe $target[$i] at Hour Angle=$tarha[$i] and EL=$tarel[$i] \n";
    }
}
