curPath=$(dirname $0)

for file in $curPath/obj/Client/*.o
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

if [ ! -d $curPath'/obj/Client' ]
then
    mkdir $curPath'/obj/Client'
fi

if [ ! -d $curPath'/bin' ]
then
    mkdir $curPath'/bin'
fi

if [ ! -d $curPath'/bin/Client' ]
then
    mkdir $curPath'/bin/Client'
fi

for file in $curPath/src/Client/*.cpp
do
    filename=${file##*/}
    filename=${filename%.*}
    g++ -c $file -o $curPath'/obj/Client/'$filename'.o' -I $curPath'/include'
done

obj=''

for file in $curPath/obj/Utils/*.o
do
    obj=$obj' '$file
done

for file in $curPath/obj/Client/*.o
do
    obj=$obj' '$file
done

g++ -o $curPath/bin/Client/PutPackClient $obj