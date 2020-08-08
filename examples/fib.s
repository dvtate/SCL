
// recursive fibbonacci
let fib = (:
	let n = i
	if (n < 2, n, (: fib(n-1) + fib(n-2)))
)

// recursive definition of while loop xd
let while = (:
	let cond = i[0], act = i[1]
	if (cond(o), (:
		act(o)
		while(cond, act)
	))
)

// Prompt for number
print('Enter a number')
let max_n = Num(input())

// Count up in fibonnacci pairs
let n = 0;
while ((: n < max_n ), (:
	print("fib(" + n + ") => " + fib(n))		
	n = n + 1
))
