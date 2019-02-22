var input(s, maxlength){
	var length;
	var b;
	var b2;
	var char;

	length = 0;
	b = 1;
	while(b){
		char = getchar();
		if(char == 13){
			char = 0;
			b = 0;
		}
		if(char == 8 & length != 0){
			length = length - 1;
			putchar(32);
			putchar(8);
		}
		if(char != 8 | length == 0){
			b2 = 1;
			if(length < maxlength - 1){
				s[length] = char;
				length = length + 1;
				b2 = 0;
			}
			if(b2){
				putchar(8);
				putchar(32);
				putchar(8);
			}
		}
	}

	s[maxlength - 1] = 0;
}

var print(s){
	while(*s){
		putchar(*s);
		s = s + 4;
	}
}

var main(){
	var string[25];
	
	input(string, 25);
	print(string);
}
