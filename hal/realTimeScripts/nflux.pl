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
# Modifications on September 19, 2005 by Brooks:
# -- I checked all of the sources against hal9000's SMA catalog, and either 
# typos were corrected or the sources were flagged -1 (do not observe)
# -- I replaced the fixed 50 deg. Sun avoidance angle with a variable 
# $SUNANGLE that can be set by the observer.  (50 deg. was too inefficient)
# -- I changed the minimum elev. angle limit $ELLLIMIT from 20 to 30 degrees.
###########################################################################

############################################################
# Specify HourAngle, and Calibrator Here in J2000 NAME !!! #
############################################################
$HAULIMIT=4.0;   # Upper Hour Angle Limit
#$HAULIMIT=0.0;   # Upper Hour Angle Limit
# $HALLIMIT=0.0;   # Lower Hour Angle Limit
$HALLIMIT=-5.0;  # Lower Hour Angle Limit

#$calib="0927+390";
#$calib="2232+117";
#$calib="0423-013";
#$calib="0359+509";
#$calib="1751+096";
#$calib="bllac";
 $calib="3c279";
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
$source[10]="0108+015"; $ra[10]=+17.16155; $dec[10]=+1.58342; $priority[10]=0;
$source[11]="0112+227"; $ra[11]=+18.02427; $dec[11]=+22.74411; $priority[11]=0;
$source[12]="0113+498"; $ra[12]=+18.36253; $dec[12]=+49.80668; $priority[12]=0;
$source[13]="0116-116"; $ra[13]=+19.05217; $dec[13]=-11.60429; $priority[13]=0;
$source[14]="0121+043"; $ra[14]=+20.48693; $dec[14]=+4.37354; $priority[14]=0;
$source[15]="0121+118"; $ra[15]=+20.42331; $dec[15]=+11.83067; $priority[15]=0;
$source[16]="0125-000"; $ra[16]=+21.37018; $dec[16]=-0.09888; $priority[16]=0;
$source[17]="0132-169"; $ra[17]=+23.18120; $dec[17]=-16.91348; $priority[17]=0;
$source[18]="0136+478"; $ra[18]=+24.24415; $dec[18]=+47.85808; $priority[18]=0;
$source[19]="0137-245"; $ra[19]=+24.40977; $dec[19]=-24.51497; $priority[19]=0;
$source[20]="0141-094"; $ra[20]=+25.35763; $dec[20]=-9.47880; $priority[20]=0;
$source[21]="0149+059"; $ra[21]=+27.34321; $dec[21]=+5.93155; $priority[21]=0;
$source[22]="0152+221"; $ra[22]=+28.07525; $dec[22]=+22.11881; $priority[22]=0;
$source[23]="0204+152"; $ra[23]=+31.21006; $dec[23]=+15.23640; $priority[23]=0;
$source[24]="0205+322"; $ra[24]=+31.27052; $dec[24]=+32.20836; $priority[24]=0;
$source[25]="0217+017"; $ra[25]=+34.45398; $dec[25]=+1.74714; $priority[25]=0;
$source[26]="0217+738"; $ra[26]=+34.37839; $dec[26]=+73.82573; $priority[26]=0;
$source[27]="0224+069"; $ra[27]=+36.11845; $dec[27]=+6.98982; $priority[27]=0;
$source[28]="0228+673"; $ra[28]=+37.20855; $dec[28]=+67.35084; $priority[28]=0;
$source[29]="0237+288"; $ra[29]=+39.46836; $dec[29]=+28.80250; $priority[29]=0;
$source[30]="0238+166"; $ra[30]=+39.66221; $dec[30]=+16.61647; $priority[30]=0;
$source[31]="0239+042"; $ra[31]=+39.96360; $dec[31]=+4.27261; $priority[31]=0;
$source[32]="0241-082"; $ra[32]=+40.26999; $dec[32]=-8.25576; $priority[32]=0;
$source[33]="0242+110"; $ra[33]=+40.62155; $dec[33]=+11.01687; $priority[33]=0;
$source[34]="0244+624"; $ra[34]=+41.24040; $dec[34]=+62.46848; $priority[34]=0;
$source[35]="0303+472"; $ra[35]=+45.89684; $dec[35]=+47.27119; $priority[35]=0;
$source[36]="0309+104"; $ra[36]=+47.26510; $dec[36]=+10.48787; $priority[36]=0;
$source[37]="0310+382"; $ra[37]=+47.70783; $dec[37]=+38.24829; $priority[37]=0;
$source[38]="0313+413"; $ra[38]=+48.25818; $dec[38]=+41.33366; $priority[38]=0;
$source[39]="0325+226"; $ra[39]=+51.40339; $dec[39]=+22.40012; $priority[39]=0;
$source[40]="0336+323"; $ra[40]=+54.12545; $dec[40]=+32.30815; $priority[40]=0;
$source[41]="0339-017"; $ra[41]=+54.87891; $dec[41]=-1.77661; $priority[41]=0;
$source[42]="0346+540"; $ra[42]=+56.64377; $dec[42]=+54.01642; $priority[42]=0;
$source[43]="0354+467"; $ra[43]=+58.62507; $dec[43]=+46.72188; $priority[43]=0;
$source[44]="0359+509"; $ra[44]=+59.87395; $dec[44]=+50.96393; $priority[44]=0;
$source[45]="0401+042"; $ra[45]=+60.33297; $dec[45]=+4.22623; $priority[45]=0;
$source[46]="0405-131"; $ra[46]=+61.39168; $dec[46]=-13.13714; $priority[46]=0;
$source[47]="0410+769"; $ra[47]=+62.69002; $dec[47]=+76.94592; $priority[47]=0;
$source[48]="0415+448"; $ra[48]=+63.98551; $dec[48]=+44.88046; $priority[48]=0;
$source[49]="0422+023"; $ra[49]=+65.71756; $dec[49]=+2.32415; $priority[49]=0;
$source[50]="0423+418"; $ra[50]=+65.98337; $dec[50]=+41.83409; $priority[50]=0;
$source[51]="0423-013"; $ra[51]=+65.81584; $dec[51]=-1.34252; $priority[51]=0;
$source[52]="0424+006"; $ra[52]=+66.19518; $dec[52]=+0.60176; $priority[52]=0;
$source[53]="0428-379"; $ra[53]=+67.16843; $dec[53]=-37.93877; $priority[53]=0;
$source[54]="0442-002"; $ra[54]=+70.66109; $dec[54]=-0.29539; $priority[54]=0;
$source[55]="0449+113"; $ra[55]=+72.28196; $dec[55]=+11.35794; $priority[55]=0;
$source[56]="0449+635"; $ra[56]=+72.34712; $dec[56]=+63.53595; $priority[56]=0;
$source[57]="0457-234"; $ra[57]=+74.26325; $dec[57]=-23.41445; $priority[57]=0;
$source[58]="0501-019"; $ra[58]=+75.30338; $dec[58]=-1.98729; $priority[58]=0;
$source[59]="0502+061"; $ra[59]=+75.56436; $dec[59]=+6.15208; $priority[59]=0;
$source[60]="0502+416"; $ra[60]=+75.65828; $dec[60]=+41.65537; $priority[60]=0;
$source[61]="0505+049"; $ra[61]=+76.34660; $dec[61]=+4.99520; $priority[61]=0;
$source[62]="0509+056"; $ra[62]=+77.35819; $dec[62]=+5.69315; $priority[62]=0;
$source[63]="0510+180"; $ra[63]=+77.50987; $dec[63]=+18.01155; $priority[63]=0;
#The next one is a B1950 name, because the J2000 name didn't work
$source[64]="0521-365"; $ra[64]=+80.74160; $dec[64]=-36.45857; $priority[64]=0;
$source[65]="0530+135"; $ra[65]=+82.73507; $dec[65]=+13.53199; $priority[65]=0;
$source[66]="0532+075"; $ra[66]=+83.16249; $dec[66]=+7.54537; $priority[66]=0;
$source[67]="0533+483"; $ra[67]=+83.31611; $dec[67]=+48.38134; $priority[67]=0;
$source[68]="0539+145"; $ra[68]=+84.92653; $dec[68]=+14.56266; $priority[68]=0;
$source[69]="0555+398"; $ra[69]=+88.87836; $dec[69]=+39.81366; $priority[69]=0;
$source[70]="0605+405"; $ra[70]=+91.46190; $dec[70]=+40.50225; $priority[70]=0;
$source[71]="0607-085"; $ra[71]=+91.99875; $dec[71]=-8.58055; $priority[71]=0;
$source[72]="0609-157"; $ra[72]=+92.42063; $dec[72]=-15.71130; $priority[72]=0;
$source[73]="0646+448"; $ra[73]=+101.63344; $dec[73]=+44.85461; $priority[73]=0;
$source[74]="0710+475"; $ra[74]=+107.69210; $dec[74]=+47.53643; $priority[74]=0;
$source[75]="0717+456"; $ra[75]=+109.46605; $dec[75]=+45.63424; $priority[75]=0;
$source[76]="0721+713"; $ra[76]=+110.47270; $dec[76]=+71.34343; $priority[76]=0;
$source[77]="0730-116"; $ra[77]=+112.57963; $dec[77]=-11.68683; $priority[77]=0;
$source[78]="0733+503"; $ra[78]=+113.46884; $dec[78]=+50.36918; $priority[78]=0;
$source[79]="0738+177"; $ra[79]=+114.53081; $dec[79]=+17.70528; $priority[79]=0;
$source[80]="0739+016"; $ra[80]=+114.82514; $dec[80]=+1.61795; $priority[80]=0;
$source[81]="0741+312"; $ra[81]=+115.29460; $dec[81]=+31.20006; $priority[81]=0;
$source[82]="0748+240"; $ra[82]=+117.15045; $dec[82]=+24.00670; $priority[82]=0;
$source[83]="0750+125"; $ra[83]=+117.71686; $dec[83]=+12.51801; $priority[83]=0;
$source[84]="0753+538"; $ra[84]=+118.25577; $dec[84]=+53.88323; $priority[84]=0;
$source[85]="0757+099"; $ra[85]=+119.27768; $dec[85]=+9.94301; $priority[85]=0;
$source[86]="0808+498"; $ra[86]=+122.16527; $dec[86]=+49.84348; $priority[86]=0;
$source[87]="0808-078"; $ra[87]=+122.06473; $dec[87]=-7.85275; $priority[87]=0;
$source[88]="0811+017"; $ra[88]=+122.86128; $dec[88]=+1.78117; $priority[88]=0;
$source[89]="0818+423"; $ra[89]=+124.56667; $dec[89]=+42.37928; $priority[89]=0;
$source[90]="0823+223"; $ra[90]=+125.85316; $dec[90]=+22.38425; $priority[90]=0;
$source[91]="0824+392"; $ra[91]=+126.23118; $dec[91]=+39.27831; $priority[91]=0;
$source[92]="0824+558"; $ra[92]=+126.19682; $dec[92]=+55.87852; $priority[92]=0;
$source[93]="0825+031"; $ra[93]=+126.45974; $dec[93]=+3.15681; $priority[93]=0;
$source[94]="0830+241"; $ra[94]=+127.71703; $dec[94]=+24.18328; $priority[94]=0;
$source[95]="0831+044"; $ra[95]=+127.95365; $dec[95]=+4.49419; $priority[95]=0;
$source[96]="0836-202"; $ra[96]=+129.16340; $dec[96]=-20.28320; $priority[96]=0;
$source[97]="0841+708"; $ra[97]=+130.35152; $dec[97]=+70.89505; $priority[97]=0;
$source[98]="0854+201"; $ra[98]=+133.70365; $dec[98]=+20.10851; $priority[98]=0;
$source[99]="0903+468"; $ra[99]=+135.76663; $dec[99]=+46.85115; $priority[99]=0;
$source[100]="0914+027"; $ra[100]=+138.65797; $dec[100]=+2.76646; $priority[100]=0;
$source[101]="0920+446"; $ra[101]=+140.24358; $dec[101]=+44.69833; $priority[101]=0;
$source[102]="0921+622"; $ra[102]=+140.40096; $dec[102]=+62.26449; $priority[102]=0;
$source[103]="0927+390"; $ra[103]=+141.76256; $dec[103]=+39.03913; $priority[103]=0;
$source[104]="0937+501"; $ra[104]=+144.30137; $dec[104]=+50.14780; $priority[104]=0;
$source[105]="0948+406"; $ra[105]=+147.23058; $dec[105]=+40.66239; $priority[105]=0;
$source[106]="0956+252"; $ra[106]=+149.20781; $dec[106]=+25.25446; $priority[106]=0;
$source[107]="0957+553"; $ra[107]=+149.40910; $dec[107]=+55.38271; $priority[107]=0;
$source[108]="0958+474"; $ra[108]=+149.58197; $dec[108]=+47.41884; $priority[108]=0;
$source[109]="0958+655"; $ra[109]=+149.69685; $dec[109]=+65.56523; $priority[109]=0;
$source[110]="1014+230"; $ra[110]=+153.69610; $dec[110]=+23.02127; $priority[110]=0;
$source[111]="1037-295"; $ra[111]=+159.31700; $dec[111]=-29.56745; $priority[111]=0;
$source[112]="1041+061"; $ra[112]=+160.32151; $dec[112]=+6.17137; $priority[112]=0;
$source[113]="1043+241"; $ra[113]=+160.78765; $dec[113]=+24.14317; $priority[113]=0;
$source[114]="1044+809"; $ra[114]=+161.09610; $dec[114]=+80.91096; $priority[114]=0;
$source[115]="1048+717"; $ra[115]=+162.11508; $dec[115]=+71.72665; $priority[115]=0;
$source[116]="1048-191"; $ra[116]=+162.02759; $dec[116]=-19.15992; $priority[116]=0;
$source[117]="1051+213"; $ra[117]=+162.95329; $dec[117]=+21.33120; $priority[117]=0;
$source[118]="1058+015"; $ra[118]=+164.62335; $dec[118]=+1.56634; $priority[118]=0;
$source[119]="1058+812"; $ra[119]=+164.54806; $dec[119]=+81.24241; $priority[119]=0;
$source[120]="1103+302"; $ra[120]=+165.80542; $dec[120]=+30.24519; $priority[120]=0;
$source[121]="1104+382"; $ra[121]=+166.11381; $dec[121]=+38.20883; $priority[121]=0;
$source[122]="1118+125"; $ra[122]=+169.73875; $dec[122]=+12.57826; $priority[122]=0;
$source[123]="1127-189"; $ra[123]=+171.76830; $dec[123]=-18.95484; $priority[123]=0;
$source[124]="1130-148"; $ra[124]=+172.52939; $dec[124]=-14.82427; $priority[124]=0;
$source[125]="1146+399"; $ra[125]=+176.74291; $dec[125]=+39.97620; $priority[125]=0;
$source[126]="1153+809"; $ra[126]=+178.30208; $dec[126]=+80.97477; $priority[126]=0;
$source[127]="1153+495"; $ra[127]=+178.35195; $dec[127]=+49.51912; $priority[127]=0;
$source[128]="1159+292"; $ra[128]=+179.88264; $dec[128]=+29.24551; $priority[128]=0;
$source[129]="1203+480"; $ra[129]=+180.87440; $dec[129]=+48.05379; $priority[129]=0;
$source[130]="1215-175"; $ra[130]=+183.94480; $dec[130]=-17.52928; $priority[130]=0;
$source[131]="1222+042"; $ra[131]=+185.59396; $dec[131]=+4.22105; $priority[131]=0;
$source[132]="1224+213"; $ra[132]=+186.22691; $dec[132]=+21.37955; $priority[132]=0;
$source[133]="1239+075"; $ra[133]=+189.85245; $dec[133]=+7.50477; $priority[133]=0;
$source[134]="1246-257"; $ra[134]=+191.69501; $dec[134]=-25.79702; $priority[134]=0;
$source[135]="1254+116"; $ra[135]=+193.65940; $dec[135]=+11.68497; $priority[135]=0;
$source[136]="1309+119"; $ra[136]=+197.39138; $dec[136]=+11.90682; $priority[136]=0;
$source[137]="1310+323"; $ra[137]=+197.61943; $dec[137]=+32.34549; $priority[137]=0;
$source[138]="1316-336"; $ra[138]=+199.03328; $dec[138]=-33.64977; $priority[138]=0;
$source[139]="1327+221"; $ra[139]=+201.75359; $dec[139]=+22.18060; $priority[139]=0;
$source[140]="1329+319"; $ra[140]=+202.47027; $dec[140]=+31.90307; $priority[140]=0;
$source[141]="1337-129"; $ra[141]=+204.41576; $dec[141]=-12.95686; $priority[141]=0;
$source[142]="1357+193"; $ra[142]=+209.26849; $dec[142]=+19.31871; $priority[142]=0;
$source[143]="1357+767"; $ra[143]=+209.48072; $dec[143]=+76.72251; $priority[143]=0;
$source[144]="1415+133"; $ra[144]=+213.99508; $dec[144]=+13.33992; $priority[144]=0;
$source[145]="1419+543"; $ra[145]=+214.94415; $dec[145]=+54.38744; $priority[145]=0;
$source[146]="1419+383"; $ra[146]=+214.94423; $dec[146]=+38.36347; $priority[146]=0;
$source[147]="1446+173"; $ra[147]=+221.64728; $dec[147]=+17.35210; $priority[147]=0;
$source[148]="1458+042"; $ra[148]=+224.74732; $dec[148]=+4.27050; $priority[148]=0;
$source[149]="1504+104"; $ra[149]=+226.10408; $dec[149]=+10.49422; $priority[149]=0;
$source[150]="1505+034"; $ra[150]=+226.27699; $dec[150]=+3.44189; $priority[150]=0;
$source[151]="1506+426"; $ra[151]=+226.72101; $dec[151]=+42.65640; $priority[151]=0;
$source[152]="1507-168"; $ra[152]=+226.76995; $dec[152]=-16.87507; $priority[152]=0;
$source[153]="1510-057"; $ra[153]=+227.72330; $dec[153]=-5.71873; $priority[153]=0;
$source[154]="1512-090"; $ra[154]=+228.21055; $dec[154]=-9.09995; $priority[154]=0;
$source[155]="1516+002"; $ra[155]=+229.16757; $dec[155]=+0.25053; $priority[155]=0;
$source[156]="1517-243"; $ra[156]=+229.42422; $dec[156]=-24.37208; $priority[156]=0;
$source[157]="1540+147"; $ra[157]=+235.20622; $dec[157]=+14.79608; $priority[157]=0;
$source[158]="1549+506"; $ra[158]=+237.32279; $dec[158]=+50.63494; $priority[158]=0;
$source[159]="1549+026"; $ra[159]=+237.37265; $dec[159]=+2.61699; $priority[159]=0;
$source[160]="1550+054"; $ra[160]=+237.64695; $dec[160]=+5.45290; $priority[160]=0;
$source[161]="1608+104"; $ra[161]=+242.19251; $dec[161]=+10.48549; $priority[161]=0;
$source[162]="1613+342"; $ra[162]=+243.42110; $dec[162]=+34.21331; $priority[162]=0;
$source[163]="1625-254"; $ra[163]=+246.44538; $dec[163]=-25.46065; $priority[163]=0;
$source[164]="1626-298"; $ra[164]=+246.52509; $dec[164]=-29.85749; $priority[164]=0;
$source[165]="1632+825"; $ra[165]=+248.13321; $dec[165]=+82.53789; $priority[165]=0;
$source[166]="1635+381"; $ra[166]=+248.81455; $dec[166]=+38.13458; $priority[166]=0;
$source[167]="1637+472"; $ra[167]=+249.43805; $dec[167]=+47.29273; $priority[167]=0;
$source[168]="1638+573"; $ra[168]=+249.55607; $dec[168]=+57.33999; $priority[168]=0;
$source[169]="1640+397"; $ra[169]=+250.12347; $dec[169]=+39.77945; $priority[169]=0;
$source[170]="1642+689"; $ra[170]=+250.53270; $dec[170]=+68.94438; $priority[170]=0;
$source[171]="1653+397"; $ra[171]=+253.46757; $dec[171]=+39.76017; $priority[171]=0;
$source[172]="1658+476"; $ra[172]=+254.51158; $dec[172]=+47.63034; $priority[172]=0;
$source[173]="1658+076"; $ra[173]=+254.53755; $dec[173]=+7.69098; $priority[173]=0;
$source[174]="1700-261"; $ra[174]=+255.22147; $dec[174]=-26.18103; $priority[174]=0;
$source[175]="1707+018"; $ra[175]=+256.89340; $dec[175]=+1.81269; $priority[175]=0;
$source[176]="1716+686"; $ra[176]=+259.05807; $dec[176]=+68.61076; $priority[176]=0;
$source[177]="1719+177"; $ra[177]=+259.80437; $dec[177]=+17.75179; $priority[177]=0;
$source[178]="1727+455"; $ra[178]=+261.86521; $dec[178]=+45.51104; $priority[178]=0;
$source[179]="1728+044"; $ra[179]=+262.10397; $dec[179]=+4.45136; $priority[179]=0;
$source[180]="1734+389"; $ra[180]=+263.58575; $dec[180]=+38.96429; $priority[180]=0;
$source[181]="1737+063"; $ra[181]=+264.30721; $dec[181]=+6.35098; $priority[181]=0;
$source[182]="1739+499"; $ra[182]=+264.86413; $dec[182]=+49.91760; $priority[182]=0;
$source[183]="1740+521"; $ra[183]=+265.15408; $dec[183]=+52.19539; $priority[183]=0;
$source[184]="1743-038"; $ra[184]=+265.99523; $dec[184]=-3.83462; $priority[184]=0;
$source[185]="1744-312"; $ra[185]=+266.09826; $dec[185]=-31.27666; $priority[185]=0;
$source[186]="1751+096"; $ra[186]=+267.88675; $dec[186]=+9.65020; $priority[186]=1;
#  The following source name is B1950 because the J2000 didn't work.
$source[187]="1751+288"; $ra[187]=+268.42700; $dec[187]=+28.80136; $priority[187]=0;
$source[188]="1800+388"; $ra[188]=+270.10319; $dec[188]=+38.80853; $priority[188]=0;
$source[189]="1800+440"; $ra[189]=+270.38465; $dec[189]=+44.07275; $priority[189]=0;
$source[190]="1800+784"; $ra[190]=+270.19035; $dec[190]=+78.46778; $priority[190]=0;
$source[191]="1801+440"; $ra[191]=+270.38465; $dec[191]=+44.07275; $priority[191]=0;
$source[192]="1824+568"; $ra[192]=+276.02945; $dec[192]=+56.85041; $priority[192]=0;
$source[193]="1830+063"; $ra[193]=+277.52475; $dec[193]=+6.32109; $priority[193]=0;
$source[194]="1833-210"; $ra[194]=+278.41619; $dec[194]=-21.06127; $priority[194]=1;
$source[195]="1842+681"; $ra[195]=+280.64017; $dec[195]=+68.15701; $priority[195]=0;
$source[196]="1848+323"; $ra[196]=+282.09205; $dec[196]=+32.31737; $priority[196]=0;
$source[197]="1848+327"; $ra[197]=+282.14318; $dec[197]=+32.73335; $priority[197]=0;
$source[198]="1849+670"; $ra[198]=+282.31697; $dec[198]=+67.09491; $priority[198]=0;
$source[199]="1911-201"; $ra[199]=+287.79022; $dec[199]=-20.11531; $priority[199]=1;
$source[200]="1924+156"; $ra[200]=+291.16440; $dec[200]=+15.67887; $priority[200]=0;
$source[201]="1924-292"; $ra[201]=+291.21273; $dec[201]=-29.24170; $priority[201]=1;
$source[202]="1925+211"; $ra[202]=+291.49835; $dec[202]=+21.10727; $priority[202]=0;
$source[203]="1927+612"; $ra[203]=+291.87678; $dec[203]=+61.29241; $priority[203]=0;
$source[204]="1927+739"; $ra[204]=+291.95206; $dec[204]=+73.96710; $priority[204]=0;
$source[205]="1955+515"; $ra[205]=+298.92808; $dec[205]=+51.53015; $priority[205]=0;
$source[206]="2000-178"; $ra[206]=+300.23788; $dec[206]=-17.81602; $priority[206]=1;
$source[207]="2005+778"; $ra[207]=+301.37916; $dec[207]=+77.87868; $priority[207]=0;
$source[208]="2007+404"; $ra[208]=+301.93727; $dec[208]=+40.49683; $priority[208]=0;
$source[209]="2009+724"; $ra[209]=+302.46792; $dec[209]=+72.48871; $priority[209]=0;
$source[210]="2011-157"; $ra[210]=+302.81546; $dec[210]=-15.77785; $priority[210]=0;
$source[211]="2012+464"; $ra[211]=+303.02349; $dec[211]=+46.48216; $priority[211]=0;
$source[212]="2015+371"; $ra[212]=+303.86972; $dec[212]=+37.18320; $priority[212]=0;
$source[213]="2016+165"; $ra[213]=+304.05775; $dec[213]=+16.54282; $priority[213]=0;
$source[214]="2023+318"; $ra[214]=+305.82924; $dec[214]=+31.88397; $priority[214]=0;
$source[215]="2025+337"; $ra[215]=+306.29518; $dec[215]=+33.71673; $priority[215]=0;
$source[216]="2049+100"; $ra[216]=+312.44110; $dec[216]=+10.05399; $priority[216]=0;
$source[217]="2101+036"; $ra[217]=+315.41181; $dec[217]=+3.69203; $priority[217]=0;
$source[218]="2109+355"; $ra[218]=+317.38282; $dec[218]=+35.54933; $priority[218]=0;
$source[219]="2123+055"; $ra[219]=+320.93549; $dec[219]=+5.58947; $priority[219]=0;
$source[220]="2131-021"; $ra[220]=+323.54296; $dec[220]=-1.88812; $priority[220]=0;
$source[221]="2134-018"; $ra[221]=+323.54296; $dec[221]=-1.88812; $priority[221]=0;
$source[222]="2136+006"; $ra[222]=+324.16078; $dec[222]=+0.69839; $priority[222]=0;
$source[223]="2139+143"; $ra[223]=+324.75545; $dec[223]=+14.39333; $priority[223]=0;
$source[224]="2147+094"; $ra[224]=+326.79235; $dec[224]=+9.49630; $priority[224]=0;
$source[225]="2148+069"; $ra[225]=+327.02275; $dec[225]=+6.96072; $priority[225]=0;
$source[226]="2152+175"; $ra[226]=+328.10341; $dec[226]=+17.57717; $priority[226]=0;
$source[227]="2158-150"; $ra[227]=+329.52617; $dec[227]=-15.01926; $priority[227]=1;
$source[228]="2203+317"; $ra[228]=+330.81240; $dec[228]=+31.76063; $priority[228]=0;
$source[229]="2203+174"; $ra[229]=+330.86206; $dec[229]=+17.43007; $priority[229]=0;
$source[230]="2213-254"; $ra[230]=+333.26041; $dec[230]=-25.49169; $priority[230]=0;
$source[231]="2217+243"; $ra[231]=+334.25343; $dec[231]=+24.36278; $priority[231]=0;
$source[232]="2218-035"; $ra[232]=+334.71683; $dec[232]=-3.59358; $priority[232]=0;
$source[233]="2229-085"; $ra[233]=+337.41702; $dec[233]=-8.54845; $priority[233]=1;
$source[234]="2232+117"; $ra[234]=+338.15170; $dec[234]=+11.73081; $priority[234]=1;
$source[235]="2236+284"; $ra[235]=+339.09363; $dec[235]=+28.48261; $priority[235]=0;
$source[236]="2246-121"; $ra[236]=+341.57597; $dec[236]=-12.11424; $priority[236]=0;
$source[237]="2258-279"; $ra[237]=+344.52485; $dec[237]=-27.97257; $priority[237]=0;
$source[238]="2301+374"; $ra[238]=+345.36557; $dec[238]=+37.44701; $priority[238]=0;
$source[239]="2311+344"; $ra[239]=+347.77220; $dec[239]=+34.41969; $priority[239]=0;
$source[240]="2320+052"; $ra[240]=+350.18690; $dec[240]=+5.23054; $priority[240]=0;
$source[241]="2323-032"; $ra[241]=+350.88314; $dec[241]=-3.28473; $priority[241]=0;
$source[242]="2333-237"; $ra[242]=+353.48016; $dec[242]=-23.72796; $priority[242]=0;
$source[243]="2334+076"; $ra[243]=+353.55345; $dec[243]=+7.60765; $priority[243]=0;
$source[244]="2348-165"; $ra[244]=+357.01087; $dec[244]=-16.52001; $priority[244]=0;
$source[245]="3c84"; $ra[245]=+49.95067; $dec[245]=+41.51170; $priority[245]=0;
$source[246]="3c111"; $ra[246]=+64.58883; $dec[246]=+38.02662; $priority[246]=0;
$source[247]="3c120"; $ra[247]=+68.29623; $dec[247]=+5.35434; $priority[247]=0;
$source[248]="3c147"; $ra[248]=+85.65058; $dec[248]=+49.85201; $priority[248]=0;
$source[249]="3c207"; $ra[249]=+130.19829; $dec[249]=+13.20654; $priority[249]=0;
$source[250]="3c273"; $ra[250]=+187.27792; $dec[250]=+2.05239; $priority[250]=0; 
$source[251]="3c274"; $ra[251]=+187.70593; $dec[251]=+12.39112; $priority[251]=0;
$source[252]="3c279"; $ra[252]=+194.04653; $dec[252]=-5.78931; $priority[252]=0;
$source[253]="3c286"; $ra[253]=+202.78453; $dec[253]=+30.50916; $priority[253]=0;
$source[254]="3c309.1"; $ra[254]=+224.78160; $dec[254]=+71.67219; $priority[254]=0;
$source[255]="3c345"; $ra[255]=+250.74504; $dec[255]=+39.81028; $priority[255]=0;
$source[256]="3c371"; $ra[256]=+271.71117; $dec[256]=+69.82447; $priority[256]=0;
$source[257]="3c380"; $ra[257]=+277.38243; $dec[257]=+48.74616; $priority[257]=0;
$source[258]="3c395"; $ra[258]=+285.73308; $dec[258]=+31.99492; $priority[258]=0;
$source[259]="3c418"; $ra[259]=+309.65431; $dec[259]=+51.32018; $priority[259]=0;
$source[260]="3c446"; $ra[260]=+336.44691; $dec[260]=-4.95039; $priority[260]=1;
$source[261]="3c454.3"; $ra[261]=+343.49062; $dec[261]=+16.14821; $priority[261]=1;
$source[262]="bllac"; $ra[262]=+330.68038; $dec[262]=+42.27777; $priority[262]=1;
$source[263]="nrao530"; $ra[263]=+263.26128; $dec[263]=-13.08043; $priority[263]=1;
#$source[264]="ngc315"; $ra[264]=+14.45368; $dec[264]=+30.35245; $priority[264]=0;
$source[264]="0057+303"; $ra[264]=+14.45368; $dec[264]=+30.35245; $priority[264]=0;



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
$ELLLIMIT=27.0;  # Lower Elevation Limit of the SMA
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
