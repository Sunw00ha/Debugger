#
# operators with mix of ints and reals -- semantic error
#
x = 100      
y = x - 140.9
z = y         
a = 1 - z    

print("x is:")
print(x)
print('y is:')
print(y)
print('z is:')
print(z)
print('a is:')
print(a)

z = a * x
print("and now for z:")
print(z)

test1 = x + y   
x = 1
test2 = y + x  
y = 123.123
test3 = x * y   
test4 = z / 3
x = 1000
test5 = z % 11.1
x = 4
test6 = test5 ** x

print("test variables:")
print(test1)
print(test2)
print(test3)
print(test4)
print(test5)
print(test6)

divisor = 3
x = 126.5 / divisor
y = 126.5 % divisor
print(x)
print(y)

z = fred    ## error: undefined

#
# random comment
#

i = 100
j = 200.9
b1_1 = 100 < 200.9
b1_2 = i < 200.9
b1_3 = 100 < j
b1_4 = i < j

print()
print('done')
