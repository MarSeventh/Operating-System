mkdir mydir
chmod a+rwx mydir
touch myfile
echo 2023 > myfile
mv moveme ./mydir
cp copyme ./mydir
cd mydir
mv copyme copied
cd ..
cat readme
gcc bad.c 2> err.txt
mkdir gen
cd gen
if [ $# -eq 0 ]
then
    a=1
    while [ $a -le 10 ]
    do
          touch $a.txt
	  a=$[$a+1]
    done
fi
if [ $# -gt 0 ] 
then
    a=1
    while [ $a -le $1 ]
    do
	    touch $a.txt
	    a=$[$a+1]
    done
fi
