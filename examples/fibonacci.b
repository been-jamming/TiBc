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

var fibinacci(output_pointer, size){
	var val1;
	var val2;
	var temp;

	val1 = 0;
	val2 = 1;
	
	while(size){
		*output_pointer = val1;
		temp = val1 + val2;
		val1 = val2;
		val2 = temp;
		
		size = size - 1;
		output_pointer = output_pointer + 4;
	}
}

var main(){
	var output;

	current_block = 0;
	*current_block = 0;
	current_block[1] = 253;
	current_block[2] = 0;
	
	output = malloc(20);
	fibinacci(output, 10);
	free(output);
}	
