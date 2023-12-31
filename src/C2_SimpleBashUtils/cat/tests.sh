#!/bin/bash

SUCCESS=0
FAIL=0
COUNTER=0
if [[ $1 == "1" ]]; then
    LEAKS_CHECK=1
else
    LEAKS_CHECK=0
fi

scp_command=(
    "./scp_cat"
)
sys_command=(
    "cat"
)

tests=(
    "FLAGS test_files/test_case_cat.txt"
    "FLAGS test_files/test_case_cat.txt test_files/test_1_cat.txt"
)
flags=(
    "b"
    "e"
    "n"
    "s"
    "t"
    "v"
)
manual=(
    "-s test_files/test_1_cat.txt"
    "-b -e -n -s -t -v test_files/test_1_cat.txt"
    "-b test_files/test_1_cat.txt nofile.txt"
    "-t test_files/test_3_cat.txt"
    "-n test_files/test_2_cat.txt"
    "no_file.txt"
    "-benvst nofile.txt test_files/test_1_cat.txt"
    "-n -b test_files/test_1_cat.txt"
    "-s -n -e test_files/test_4_cat.txt"
    "test_files/test_1_cat.txt -n"
    "-n test_files/test_1_cat.txt"
    "-n test_files/test_1_cat.txt test_files/test_2_cat.txt"
    "-v test_files/test_5_cat.txt"
    "-- test_files/test_5_cat.txt"
)

gnu=(
    "-T test_files/test_1_cat.txt"
    "-E test_files/test_1_cat.txt"
    "-vT test_files/test_3_cat.txt"
    "--number test_files/test_2_cat.txt"
    "--squeeze-blank test_files/test_1_cat.txt"
    "--number-nonblank test_files/test_4_cat.txt"
    "test_files/test_1_cat.txt --number --number"
    "-bnvste test_files/test_6_cat.txt"
)

run_test() {

    param=$(echo "$@" | sed "s/FLAGS/$var/")
    let "COUNTER++"
    if [ "$LEAKS_CHECK" == 1 ]; then
        # echo "valgrind --leak-check=full --show-leak-kinds=all "${scp_command[@]}" $param 2> >(grep -ic -e \"LEAK SUMMARY:\" -e \"ERROR SUMMARY: [^0]\" 2>/dev/null 1>&2)"
        valgrind --leak-check=full --show-leak-kinds=all "${scp_command[@]}" $param 2>&1 >/dev/null | grep -ic -e "LEAK SUMMARY:" -e "ERROR SUMMARY: [^0]" >/dev/null
    else
        cmp -s <("${scp_command[@]}" $param 2>/dev/null) <("${sys_command[@]}" $param 2>/dev/null)
    fi

    res=$?
    if [ $LEAKS_CHECK == $res ]; then
        let "SUCCESS++"
        echo "$COUNTER - Success $param"
    else
        let "FAIL++"
        echo "$COUNTER - Fail $param"
    fi
}

echo "^^^^^^^^^^^^^^^^^^^^^^^"
echo -n "TESTS WITH NORMAL FLAGS" && [ $LEAKS_CHECK == 1 ] && echo " AND VALGRIND" || echo
echo "^^^^^^^^^^^^^^^^^^^^^^^"
printf "\n"
echo "#######################"
echo "MANUAL TESTS"
echo "#######################"
printf "\n"
for i in "${manual[@]}"; do
    var="-"
    run_test "$i"
done
printf "\n"
echo "#######################"
echo "AUTOTESTS"
echo "#######################"
printf "\n"
echo "======================="
echo "1 PARAMETER"
echo "======================="
printf "\n"

for var1 in "${flags[@]}"; do
    for i in "${tests[@]}"; do
        var="-$var1"
        run_test "$i"
    done
done
printf "\n"
echo "======================="
echo "2 PARAMETERS"
echo "======================="
printf "\n"

for var1 in "${flags[@]}"; do
    for var2 in "${flags[@]}"; do
        if [ $var1 != $var2 ]; then
            for i in "${tests[@]}"; do
                var="-$var1 -$var2"
                run_test "$i"
            done
        fi
    done
done
printf "\n"
echo "======================="
echo "3 PARAMETERS"
echo "======================="
printf "\n"
for var1 in "${flags[@]}"; do
    for var2 in "${flags[@]}"; do
        for var3 in "${flags[@]}"; do
            if [ $var1 != $var2 ] && [ $var2 != $var3 ] && [ $var1 != $var3 ]; then
                for i in "${tests[@]}"; do
                    var="-$var1 -$var2 -$var3"
                    run_test "$i"
                done
            fi
        done
    done
done
printf "\n"
echo "======================="
echo "4 PARAMETERS"
echo "======================="
printf "\n"
for var1 in "${flags[@]}"; do
    for var2 in "${flags[@]}"; do
        for var3 in "${flags[@]}"; do
            for var4 in "${flags[@]}"; do
                if [ $var1 != $var2 ] && [ $var2 != $var3 ] &&
                    [ $var1 != $var3 ] && [ $var1 != $var4 ] &&
                    [ $var2 != $var4 ] && [ $var3 != $var4 ]; then
                    for i in "${tests[@]}"; do
                        var="-$var1 -$var2 -$var3 -$var4"
                        run_test "$i"
                    done
                fi
            done
        done
    done
done
# 2 сдвоенных параметра
for var1 in "${flags[@]}"; do
    for var2 in "${flags[@]}"; do
        if [ $var1 != $var2 ]; then
            for i in "${tests[@]}"; do
                var="-$var1$var2"
                run_test "$i"
            done
        fi
    done
done

# 3 строенных параметра
for var1 in "${flags[@]}"; do
    for var2 in "${flags[@]}"; do
        for var3 in "${flags[@]}"; do
            if [ $var1 != $var2 ] && [ $var2 != $var3 ] && [ $var1 != $var3 ]; then
                for i in "${tests[@]}"; do
                    var="-$var1$var2$var3"
                    run_test "$i"
                done
            fi
        done
    done
done
printf "\n"
echo "FAILED: $FAIL"
echo "SUCCESSFUL: $SUCCESS"
echo "ALL: $COUNTER"
printf "\n"
##############################
echo "^^^^^^^^^^^^^^^^^^^^^^^"
echo -n "TESTS WITH GNU FLAGS" && [ $LEAKS_CHECK == 1 ] && echo " AND VALGRIND" || echo
echo "^^^^^^^^^^^^^^^^^^^^^^^"
printf "\n"
FAIL=0
SUCCESS=0
COUNTER=0
echo "======================="
echo "MANUAL TESTS"
echo "======================="
printf "\n"

for i in "${gnu[@]}"; do
    var="-"
    run_test $i
done
printf "\n"

echo "FAILED: $FAIL"
echo "SUCCESSFUL: $SUCCESS"
echo "ALL: $COUNTER"
printf "\n"

exit $FAIL