#pragma once

namespace waypoint
{

class Engine;
class Engine_impl;
auto get_impl(Engine const &engine) -> Engine_impl &;

class Group;
class Group_impl;
auto get_impl(Group const &group) -> Group_impl &;

class Test;
class Test_impl;
auto get_impl(Test const &test) -> Test_impl &;

class Context;
class Context_impl;
auto get_impl(Context const &context) -> Context_impl &;

class Result;
class Result_impl;
auto get_impl(Result const &result) -> Result_impl &;

} // namespace waypoint
