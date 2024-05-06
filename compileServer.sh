curPath=$(dirname $0)

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

g++ -o $curPath/bin/Server/PutPack.exe $obj