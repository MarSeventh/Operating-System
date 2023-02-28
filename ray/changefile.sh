a=71
while [ $a -ne 101 ]
do
	rm -rf file$a
	a=$[$a+1]
done
b=41
while [ $b -ne 71 ]
do 
	mv  file$b newfile$b
	b=$[$b+1]
done
