print('pointer assignment')
print()

x = 3.14159
y = x + 299

px = &x
py = &y

pass

*px = 4.75
testing = *px
print(testing)

i = 12
j = i ** 2

pi = &i
pj = &j

s = "this is a string, "
ps = &s

pass

*px = *px + 99.25
a = *px
print(a)

b = i - *py
print(b)

pass

*py = *ps + " with more to follow"
t = *py
print(t)

pass

*pi = *pj * *pi
c = *pi
print(c)

pz = &b

d = *px < *pz
e = *px >= *pz

pi = &c
pi = pi + 4

pass

*pi = *pj / *pi      ## semantic error 
f = *pi
print(d)
print(e)
print(f)

print()
print('done')