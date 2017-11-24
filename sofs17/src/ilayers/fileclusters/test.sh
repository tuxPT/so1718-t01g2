#!/usr/bin/bash
# Leonardo Miguel Oliveira Costa 80162

mkfifo input
mkfifo output
cat > input &
end1=$!
fifo_name="output"
cat > output &
end2=$!

./testtool -q1 testdisk < input > $fifo_name 2>&1 &

echo "Fileclusters Testing" 


declare -a func=("AllocFileCluster" "GetFileCluster" "FreeFileCluster" "WriteFileCluster" "ReadFileCluster")

declare -a array=(0 5 6 517 518 262661)
for i in "${array[@]}"
do
    echo "afc" > input
    echo "0" > input
    echo "$i" > input
done


for j in "${array[@]}"
do
    echo "gfc" > input
    echo "0" > input
    echo "$j" > input
done


for k in "${array[@]}"
do
    echo "ffc" > input
    echo "0" > input
    echo "$k" > input
done


for l in "${array[@]}"
do
    echo "wfc" > input
    echo "0" > input
    echo "$l" > input
    echo "42" > input
done


for n in "${array[@]}"
do
    echo "rfc" > input
    echo "0" > input
    echo "$n" > input
done

big_count=0;
array_count=0
big5=0
while read -r line
do
    d=$(echo "$line" | grep -i "cluster number\|operation\|0:")
    if [ "$d" != "" ]; then  
        if ((big_count==4)); then
            if ((big5==0)); then
                if ((array_count==0)); then
                    echo -e "\n${func["$big_count"]}";
                fi  
                echo -e "\n### cluster file number ${array["$array_count"]} ###"
                array_count=$((array_count+1))
                array_count=$((array_count%6))
            fi
            echo "$line" | grep -i "0:"
            big5=$((big5+1))
            big5=$((big5%64))
        else
            if ((array_count==0 && big_count!=4)); then
                echo -e "\n${func["$big_count"]}";
                big_count=$((big_count + 1));
            fi  
            echo "### cluster file number ${array["$array_count"]} ###"
            array_count=$((array_count+1))
            array_count=$((array_count%6))
            echo "$line" | grep -i "cluster number\|operation"
        fi
        
    fi
done <"$fifo_name"

kill -9 "$end1"
kill -9 "$end2"
killall testtool