var fibinacci(output_pointer, size){
	var val1;
	var val2;
	var temp;
	
	while(size){
		*output_pointer = val1;
		temp = val1 + val2;
		val1 = val2;
		val2 = temp;
		
		size = size - 1;
		output_pointer = output_pointer + 1;
	}
}
