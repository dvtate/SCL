let time = import('libtime.so')

// Useful stuff
let while = (:
	let cond = i[0], act = i[1], break = o
	if (cond(break), (:
		act(break)
		while(cond, act)
	))
)
let foreach = (:
	let list = i[0], action = i[1]
	let index = 0, end = size(list)
	while((: index < end), (:
		action(list[index], index, i)
		index = index + 1	
	))
)
let map = (:
	let list = i[0], fn = i[1]
	let list = copy(i[0])
	let ret = foreach(i[0], (: ret[i[1]] = fn(i)))
	if(ret == empty, list, ret)
)

// Game State
let world = [
	"                                   ",
	"    ###       #                    ",
	"   ###       # #           #       ",
	"              #            #       ",
	"    ##                     #       ",
	"    #                ##            ",
	"     ###             ##            ",
	"       #               ##          ",
	"                       ##          "
]

// Standins for proper shortcircuit operators
let or =  (: if (i[0], i[0], i[1]) )
let and = (: if (i[0], i[1], 0   ) )

// Get number of neighbors for cell at given coord
let neighbors = (:
	let x = i[0], y = i[1]
	
	let s = and(world[x + 1], (: world[x + 1][y] == '#' ))
	let n = and(x > 0, (: world[x - 1][y] == '#' ))
	let w = world[x][y - 1] == '#'
	let e = world[x][y + 1] == '#'
	let se = and(world[x + 1], (: world[x + 1][y + 1] == '#' ))
	let sw = and(world[x + 1], (: world[x + 1][y - 1] == '#'))
	let ne = and(x > 0, (: world[x - 1][y + 1] == '#' ))
	let nw = and(x > 0, (: world[x - 1][y - 1] == '#' ))

	let ret = 0
	foreach([n, s, e, w, ne, nw, se, sw], (:
		ret = ret + Num(i[0])
	))
	o(ret)
)

let print_world = (: 
	foreach(world, (:
		print(i[0])
	))
);

// Update the world based on rules of Conways game of life
let update_world = (:
	let ret = copy(world);
	foreach (world, (:
		let row = i[1]
		ret[row] = ""
		foreach(world[row], (:
			let col = i[1]
			let ns = neighbors(row, col)
			ret[row] = ret[row] + if (
				or(
					and(i[0] == ' ', ns == 3),
					and(i[0] == '#', or(ns == 2, ns == 3))),
				'#',
				' '
			)
		))
	))
	world = ret
);

// main loop
while ((: true ), (:
	print_world()
	update_world()
	time.delay(0.04)
))

