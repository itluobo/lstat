local lstat = require('stat')
-- local a = "hahahah"

-- local function co_func()
-- 	print(a)
-- end

-- local function co_test()
-- 	print(a)
-- 	co_func()
-- 	coroutine.yield(1)
-- 	co_func()
-- end

-- local co

-- local function foo1()
-- 	return print('leave foo')
-- end

-- local function foo()
-- 	foo1()
-- end

-- local function test()
-- 	print("---->enter test func")
-- 	return foo()
-- end

-- local function run()
-- 	co = coroutine.create(co_test)
-- 	lstat.link_co(co)
-- 	coroutine.resume(co)
-- 	print(test())
-- 	coroutine.resume(co)
-- end

local function function_name( ... )
	local x = 0
	for i = 1, 1000000 do
		x = x + 1
	end
end

local function co_foo()
	for i = 1, 100 do
		function_name()
	end
end

local function co_func()
	co_foo()
	print("co_func hehe")
end

local function func1()
	local x = 0
	for i = 1, 1000000 do
		x = x + 1
	end
end

local function func2()
	for i = 1, 5 do
		func1()
	end
end

local function func3()
	for i = 1, 5 do
		func1()
	end
	for i = 1, 5 do
		func2()
	end
end

local function run()
	for i = 1, 5 do
		func1()
	end
	for i = 1, 5 do
		func2()
	end
	local co = coroutine.create(co_func)
	--lstat.link_co(co)
	coroutine.resume(co)
	for i = 1, 5 do
		func3()
	end
end

local function main()
	run()
end

print(lstat.stat(main))
