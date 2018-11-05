var collatz(n){
	var count;
	var b;

	count = 0;

	while(n > 1 & count < 10000){
		b = 1;
		if(n&1){
			n = 3*n + 1;
			b = 0;
		}
		
		if(b){
			n = n/2;
		}
		count = count + 1;
	}

	return n;
}

var main(){
	var n;
	
	while(n < 10000){
		if(collatz(n) > 1){
			return n;
		}

		n = n + 1;
	}

	return 0;
}
