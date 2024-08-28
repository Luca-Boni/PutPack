curPath=$(dirname $0)

for file in $curPath/obj/Server/*.o
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

if [ ! -d $curPath'/obj/Server' ]
then
    mkdir $curPath'/obj/Server'
fi

if [ ! -d $curPath'/bin' ]
then
    mkdir $curPath'/bin'
fi

if [ ! -d $curPath'/bin/Server' ]
then
    mkdir $curPath'/bin/Server'
fi

for file in $curPath/src/Server/*.cpp
do
    filename=${file##*/}
    filename=${filename%.*}
    g++ -c $file -o $curPath'/obj/Server/'$filename'.o' -I $curPath'/include'
done

obj=''

for file in $curPath/obj/Utils/*.o
do
    obj=$obj' '$file
done

for file in $curPath/obj/Server/*.o
do
    obj=$obj' '$file
done

g++ -o $curPath/bin/Server/PutPackServer $obj