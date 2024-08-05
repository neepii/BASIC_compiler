

test() {
    echo -e "$1" > test1.bas
    ./exec test1.bas >> /dev/null
    ./a.out
    if [ $? -ne 0 ];  then
	echo $1 "this test failed"
	exit 1
    else
	echo "($SECONDS)s" ":" "passed"
    fi
    rm test1.bas a.out
}

test "
10 LET A = 5 + 5*8\n
20 PRINT A\n
30 END"

test "
10 PRINT \"HI\"\n
20 END
"

test "
10 LET A = 8 * 9 + 41\n
20 print a\n
30 LET B = 9 * 2\n
40 print b\n
50 end
"

test "
10 LET A = 8 * 9 + 41\n
20 LET B = 9 * 2\n
30 print b\n
40 print a\n
50 print b\n
60 end
"

test "
10 PRINT 20 + 20\n
20 END
"
test "10 INPUT A\n
20 PRINT A\n
30 END
"
