curPath=$(dirname $0)

for file in $curPath/obj/Utils/*.o
do
    rm $file
done

if [ ! -d $curPath'/obj' ]
then
    mkdir $curPath'/obj'
fi

if [ ! -d $curPath'/obj/Utils' ]
then
    mkdir $curPath'/obj/Utils'
fi

for file in $curPath/src/Utils/*.cpp
do
    filename=${file##*/}
    filename=${filename%.*}
    g++ -c $file -o $curPath'/obj/Utils/'$filename'.o' -I $curPath'/include'
done