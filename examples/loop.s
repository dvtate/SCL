// recursive definition of while loop xd
let while = (:
	let cond = i[0], act = i[1];
	print('while', [cond, act]);
	if (cond(o), (:
		act(o);
		while(cond, act);
	));
);

let n = 0;
while ((: n < 6 ), (:
    print(n = n + 1)
));

