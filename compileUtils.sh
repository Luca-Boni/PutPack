curPath=$(dirname $0)

for file in $curPath/src/Utils/*.cpp
do
    filename=${file##*/}
    filename=${filename%.*}
    g++ -c $file -o $curPath'/obj/Utils/'$filename'.o' -I $curPath'/include'
done