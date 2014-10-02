echo "observe -s 1751+096"
observe -s 1751+096
echo "tsys"
tsys
echo "integrate -s 7 -w"
integrate -s 7 -w
echo "observe -s callisto" 
observe -s callisto
echo "tsys"
tsys
echo "integrate -s 20 -w"
integrate -s 20 -w
echo "observe -s vesta"
observe -s vesta  
echo "tsys"
tsys
echo "integrate -s 20 -w"
integrate -s 20 -w
echo "bring nflux script to foreground"
fg

