var current_block;

var malloc(n){
	var first_block;
	var next_block;
	var b;

	first_block = current_block;
	current_block = *current_block;

	b = current_block[1] < n | current_block[2];
	while(b & current_block != first_block){
		next_block = *current_block;
		while(current_block[2] == 0 & next_block[2] == 0 & next_block != first_block){
			*current_block = *next_block;
			current_block[1] = current_block[1] + next_block[1] + 3;
			next_block = *next_block;
		}

		current_block = next_block;
		b = current_block[1] < n | current_block[2];
	}
	
	if(b){
		return 0;
	}
	
	current_block[2] = 1;
	if(current_block[1] > n + 12){
		current_block[n + 3] = *current_block;
		current_block[n + 4] = current_block[1] - n - 3;
		current_block[n + 5] = 0;
		current_block[1] = n;
		*current_block = current_block + n + n + n + n + 12;
	}
	
	return current_block + 12;
}

var free(p){
	var next_block;
	current_block = p - 12;
	p[-1] = 0;
	
	next_block = p[-3];
	while(next_block[2] == 0 & next_block + 12 != p){
		p[-3] = *next_block;
		p[-2] = p[-2] + next_block[1] + 3;
		next_block = *next_block;
	}
}

var main(){
	var a;
	var b;
	var c;

	current_block = 0;
	*current_block = 0;
	current_block[1] = 253;
	current_block[2] = 0;

	a = malloc(2);
	a[1] = 221;
	b = malloc(3);
	c = malloc(10);
	free(c);
	free(b);
	b = malloc(13);
}
