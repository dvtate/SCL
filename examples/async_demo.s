// Import a native module
let time = import('libtime.so')

// This is equivalent to JavaScript's window.setTimeout
let set_timeout = (:
	let duration = i[0], action = i[1], arg = i[2]
	async((:
		time.delay(duration)
		action(arg)
	))()
)

// Some different ways of calling set timeout...	
set_timeout(3000, print, "Hello 1")
set_timeout(2000, (: print("Hello 2") ))
async((:
	time.delay(1000)
	print("Hello 3")
))()

// Function returns after given number of seconds :)
let delay_wrap = (:set_timeout(i, o)())

let delay_test = (: set_timeout(i, o); o = 0; );



// wait for other threads to complete
delay_wrap(3100)
print('--------')
