var factorial(n){
	if(n < 2){
		return 1;
	}

	return n*factorial(n - 1);
}

var factorial2(n){
	var result;
	result = 1;

	while(n > 1){
		result = result*n;
		n = n - 1;
	}

	return result;
}
