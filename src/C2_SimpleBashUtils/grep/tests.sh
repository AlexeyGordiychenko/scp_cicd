#!/bin/bash

SUCCESS=0
FAIL=0
COUNTER=0
if [[ $1 == "1" ]]; then
    LEAKS_CHECK=1
else
    LEAKS_CHECK=0
fi

s21_command=(
    "./s21_grep"
)
sys_command=(
    "grep"
)

flags=(
    "i"
    "v"
    "c"
    "l"
    "n"
    "h"
    "o"
)

tests=(
    "s test_files/test_0_grep.txt FLAGS"
    "for grep.c grep.h Makefile FLAGS"
    "for grep.c FLAGS"
    "-e for -e ^int grep.c grep.h Makefile FLAGS"
    "-e for -e ^int grep.c FLAGS"
    "-e regex -e ^print grep.c FLAGS -f test_files/test_ptrn_grep.txt"
    "-e while -e void grep.c Makefile FLAGS -f test_files/test_ptrn_grep.txt"
    "-e intel -e int FLAGS test_files/test_7_grep.txt"
    "-e int -e intel FLAGS test_files/test_7_grep.txt"
)

manual=(
    ""
    "-n for test_files/test_1_grep.txt test_files/test_2_grep.txt"
    "-n for test_files/test_1_grep.txt"
    "-n -e ^\} test_files/test_1_grep.txt"
    "-c -e /\ test_files/test_1_grep.txt"
    "-ce ^int test_files/test_1_grep.txt test_files/test_2_grep.txt"
    "-e ^int test_files/test_1_grep.txt"
    "-nivh = test_files/test_1_grep.txt test_files/test_2_grep.txt"
    # "-e"
    "-ie INT test_files/test_5_grep.txt"
    "-echar test_files/test_1_grep.txt test_files/test_2_grep.txt"
    "-ne = -e out test_files/test_5_grep.txt"
    "-iv int test_files/test_5_grep.txt"
    "-in int test_files/test_5_grep.txt"
    "-c -l aboba test_files/test_1_grep.txt test_files/test_5_grep.txt"
    "-v test_files/test_1_grep.txt -e ank"
    "-noe "\)" test_files/test_5_grep.txt"
    "-l for test_files/test_1_grep.txt test_files/test_2_grep.txt"
    "-o -e int test_files/test_4_grep.txt"
    "-o -e int -e in -e i test_files/test_1_grep.txt"
    "-o -e in -e i -e int test_files/test_1_grep.txt"
    "-e = -e out test_files/test_5_grep.txt"
    "-noe ing -e as -e the -e not -e is test_files/test_6_grep.txt"
    "-e ing -e as -e the -e not -e is test_files/test_6_grep.txt"
    "-c -e . test_files/test_1_grep.txt -e '.'"
    "-l for no_file.txt test_files/test_2_grep.txt"
    "-e int -si no_file.txt grep.c no_file2.txt grep.h"
    "-si grep.c -f no_pattern.txt"
    "-f test_files/test_3_grep.txt test_files/test_5_grep.txt"
)

run_test() {

    param=$(echo "$@" | sed "s/FLAGS/$var/")
    let "COUNTER++"
    if [ "$LEAKS_CHECK" == 1 ]; then
        # echo $"valgrind --leak-check=full --show-leak-kinds=all "${s21_command[@]}" $param 2>&1 >/dev/null | grep -ic -e \"LEAK SUMMARY:\" -e \"ERROR SUMMARY: [^0]\" >/dev/null"
        valgrind --leak-check=full --show-leak-kinds=all "${s21_command[@]}" $param 2>&1 >/dev/null | grep -ic -e "LEAK SUMMARY:" -e "ERROR SUMMARY: [^0]" >/dev/null
    else
        cmp -s <(
            "${s21_command[@]}" ${param} 2>/dev/null
            echo ${PIPESTATUS[0]}
        ) <(
            "${sys_command[@]}" --color=never ${param} 2>/dev/null
            echo ${PIPESTATUS[0]}
        )
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
echo "TESTS WITH NORMAL FLAGS"
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
echo "FAILED: $FAIL"
echo "SUCCESSFUL: $SUCCESS"
echo "ALL: $COUNTER"
printf "\n"
##############################

exit $FAIL