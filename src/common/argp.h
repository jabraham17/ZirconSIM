

// #ifndef ZIRCON_COMMON_ARGP_H_
// #define ZIRCON_COMMON_ARGP_H_

// #include <exception>
// #include <string>
// #include <tuple>
// #include <unordered_map>
// #include <utility>
// #include <vector>
// #include <memory>

// namespace common {

// struct OptionalException : public std::exception {
//     const char* what() const noexcept { return "Optional has no value"; }
// };

// template <typename T> class Optional {
//   private:
//     bool has_value_;
//     T value_;

//   public:
//     Optional() : has_value_(false), value_() {}
//     Optional(T value) : has_value_(true), value_(value) {}

//     Optional operator=(T value) {
//         this->value_ = value;
//         this->has_value_ = true;
//         return *this;
//     }
//     T operator*() { return value(); }
//     T operator->() { return value(); }
//     T value() {
//         if(has_value()) return value_;
//         else throw OptionalException();
//     }
//     T value_or(T other_value) {
//         if(has_value()) return value_;
//         else return other_value;
//     }
//     operator bool() { return has_value(); }
//     bool has_value() { return has_value_; }
// };

// namespace argp {

// enum class ArgType {
//     NONE,
//     FLAG,
//     SINGLE,
// };
// // class Argument {
// //     // private: ArgType type;
// //   public:
// //     // Argument(ArgType type) :type(type){};
// //     Argument() = default;
// //     virtual ~Argument() = default;
// // };
// // // no args, just a boolean flag if present
// // class FlagArgument : public Argument {
// //   private:
// //     bool present;

// //   public:
// //     FlagArgument() : present(present) {}
// //     bool isPresent() { return present; }
// // };

// struct ArgumentDefinitionException : public std::exception {
//     std::string msg;
//     ArgumentDefinitionException(std::string msg = "Invalid Argument
//     Definition")
//         : msg(msg) {}
//     const char* what() const noexcept { return msg.c_str(); }
// };

// class Parser;

// class ArgumentBase {
//     friend Parser;

//   private:
//     Optional<std::string> shortname;
//     Optional<std::string> longname;
//     bool allow_single_dash_for_longname;
//           ArgType argType;

//   public:
//     ArgumentBase(
//         Optional<std::string> shortname,
//         Optional<std::string> longname,
//         bool allow_single_dash_for_longname = false)
//         : shortname(shortname), longname(longname),
//           allow_single_dash_for_longname(), argType(ArgType::NONE) {
//         if(!shortname && !longname)
//             throw ArgumentDefinitionException("Must specify a name");
//     }
//     virtual ~ArgumentBase() = default;

//     virtual std::vector<std::string> getPossibleNames() {
//         std::vector<std::string> names;
//         if(shortname) names.push_back("-" + *shortname);
//         if(longname) names.push_back("--" + *longname);
//         if(longname && allow_single_dash_for_longname)
//             names.push_back("-" + *longname);
//         return names;
//     }

//     void setArgType(ArgType at) {argType = at;}
//     ArgType getArgType() {return argType;}

// };

// class FlagArgument : public ArgumentBase {
//     friend Parser;

//   private:
//     bool default_value;
//     Optional<bool> value_;

//   public:
//     FlagArgument(
//         Optional<std::string> shortname,
//         Optional<std::string> longname,
//         bool default_value,
//         bool allow_single_dash_for_longname = false)
//         : ArgumentBase(shortname, longname, allow_single_dash_for_longname),
//           default_value(default_value){
//             setArgType(ArgType::FLAG);
//           }
//     virtual ~FlagArgument() = default;
//     bool value() {
//         if(!value_) return default_value;
//         else return *value_;
//     }
// };

// template<typename ArgType>
// class Argument : public ArgumentBase {
//     friend Parser;

//   private:
//     ArgType default_value;
//     Optional<ArgType> value_;

//   public:
//     Argument(
//         Optional<std::string> shortname,
//         Optional<std::string> longname,
//         ArgType default_value,
//         bool allow_single_dash_for_longname = false,
//         std::string sep = ",")
//         : ArgumentBase(shortname, longname, allow_single_dash_for_longname),
//           default_value(default_value) {
//             setArgType(ArgType::SINGLE);
//           }
//     virtual ~Argument() = default;

//     ArgType value() {
//         if(!value_) return default_value;
//         else return *value_;
//     }

// };

// // template<typename... ArgTypes>
// // class TupleArgument : public Argument {
// //     friend Parser;
// //     using TupleType = std::tuple<ArgTypes...>;

// //   private:
// //     TupleType default_value;
// //     std::string sep;
// //     Optional<TupleType> value_;

// //   public:
// //     TupleArgument(
// //         Optional<std::string> shortname,
// //         Optional<std::string> longname,
// //         TupleType default_value,
// //         bool allow_single_dash_for_longname = false,
// //         std::string sep = ",")
// //         : Argument(shortname, longname, allow_single_dash_for_longname),
// //           default_value(default_value), sep(sep) {
// //           }
// //     virtual ~TupleArgument() = default;

// //     TupleType value() {
// //         if(!value_) return default_value;
// //         else return *value_;
// //     }

// // };

// class ArgumentList {
//     friend Parser;
//     private:
//     std::vector<std::shared_ptr<ArgumentBase>> allargs_;
//     std::vector<std::shared_ptr<FlagArgument>> flags_;
//     std::vector<std::shared_ptr<FlagArgument>> singleargs_;

//   public:
//     ArgumentList& flag(
//         Optional<std::string> shortname,
//         Optional<std::string> longname,
//         bool default_value = false,
//         bool allow_single_dash_for_longname = false) {
//             auto p = std::make_shared<FlagArgument>(shortname,
//             longname,default_value,allow_single_dash_for_longname);
//             allargs_.push_back(p);
//             flags_.push_back(p);
//             return *this;
//         }
//     template <typename ArgType>
//     ArgumentList& arg(
//         Optional<std::string> shortname,
//         Optional<std::string> longname,
//         ArgType default_value,
//         bool allow_single_dash_for_longname = false) {
//              args_.push_back(std::make_shared<Argument<ArgType>>(shortname,
//              longname,default_value,allow_single_dash_for_longname));
//             return *this;
//         }
//     // template <typename ArgType>
//     // ArgumentList& arg_multiple(
//     //     Optional<std::string> shortname,
//     //     Optional<std::string> longname,
//     //     bool allow_single_dash_for_longname = false);

//     // template <typename... ArgTypes>
//     // ArgumentList& tuple_arg(
//     //     Optional<std::string> shortname,
//     //     Optional<std::string> longname,
//     //     std::tuple<ArgTypes...> default_value,
//     // std::string sep = ",",
//     //     bool allow_single_dash_for_longname = false
//     //     );
//     // template <typename... ArgTypes>
//     // ArgumentList& tuple_arg_multiple(
//     //     Optional<std::string> shortname,
//     //     Optional<std::string> longname,
//     // std::string sep = ",",
//     //     bool allow_single_dash_for_longname = false);
// };

// class Parser {

//     struct UnknownArgumentException : public std::exception {
//         std::string unknownArg;
//         UnknownArgumentException(std::string unknownArg) :
//         unknownArg(unknownArg) {}
//     const char* what() const noexcept { return "Unknown Argument"; }
// };

//     void parse(int argc, char** argv, ArgumentList al) {
//         int idx = 0;
//         while(idx < argc) {
//             std::string current = argv[idx];
//             idx++;

//             std::shared_ptr<ArgumentBase> arg = nullptr;
//             for(auto& a: al.args_) {
//                 auto possibleNames = a->getPossibleNames();
//                 if(std::find(possibleNames.begin(), possibleNames.end(),
//                 current) != possibleNames.end()) {
//                     arg = a;
//                     break;
//                 }
//             }
//             if(!arg) {
//                 throw UnknownArgumentException(current);
//             }
//             switch(arg->getArgType()) {
//                 default:
//                 case ArgType::NONE: throw ArgumentDefinitionException();
//                 case ArgType::FLAG: {
//                     auto flagArg =
//                     std::dynamic_pointer_cast<FlagArgument,ArgumentBase>(arg);
//                     // value of the flag is the opposite of the default
//                     flagArg->value_ = !flagArg->default_value;
//                     break;
//                 }
//                 case ArgType::SINGLE: {
//                     auto singleArg =
//                     std::dynamic_pointer_cast<Argument<>,ArgumentBase>(arg);
//                     if(std::is_integral<>::value)
//                     break;
//                 }

//             }
//         }
//     }
// };

// } // namespace argp

// } // namespace common
// #endif
