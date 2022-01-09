#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

struct Template
{
    std::string name;
    std::string category;
    std::vector<std::string> types;
    std::vector<std::string> content;

    Template(std::string name, std::vector<std::string> types, std::vector<std::string> content)
        : name(name), types(types), content(content)
    {
        category = name;
    }

    Template()
    {
    }
};

bool validate(std::vector<std::string> &matches, std::string str, std::regex reg, bool store)
{
    bool is_any_match = false;

    std::sregex_iterator currentMatch(str.begin(), str.end(), reg);

    std::sregex_iterator lastMatch;

    while (currentMatch != lastMatch)
    {
        is_any_match = true;
        std::smatch match = *currentMatch;

        if (store)
            matches.push_back(match.str());

        currentMatch++;
    }

    return is_any_match;
}

bool validate(std::string str, std::regex reg)
{
    std::vector<std::string> matches;
    return validate(matches, str, reg, false);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    std::ifstream file;

    file.open(argv[1], std::ios_base::binary);

    if (!file.is_open())
    {
        std::cout << "Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string name(argv[1]);
    std::string tmp = name.substr(0, name.find_last_of(".")) + "_gen.c";
    std::cout << "New file name: " << tmp << std::endl;

    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();

    std::regex templ("template[ ]*<(.*)>");
    std::regex func_def("([\\w_]+)\\((.*?)\\)");
    std::regex func_call("([\\w_]+)<(.*?)>\\((.*?)\\)");
    std::regex struct_def("struct[ ]*([\\w_]+)");
    std::regex struct_decl("([\\w_]+)<(.*)>[ ]*");

    bool is_template = false;

    std::vector<std::string> types;
    std::vector<std::string> template_args;

    std::vector<Template> templates;

    int count = 0;
    bool atleast_one_brace = false;
    bool name_found = false;
    std::string template_name;

    int line_num = 1;

    for (auto line : lines)
    {
        if (is_template)
        {
            template_args.push_back(line);

            if (!name_found && (validate(line, func_def) || validate(line, struct_def)))
            {
                name_found = true;

                std::smatch match;

                if (validate(line, func_def))
                    std::regex_search(line, match, func_def);
                else
                    std::regex_search(line, match, struct_def);

                template_name = match[1];
            }

            if (line.find("{") != std::string::npos)
            {
                count++;
                atleast_one_brace = true;
            }

            if (line.find("}") != std::string::npos)
            {
                count--;
            }

            if (template_args.size() != 0 && count == 0 && atleast_one_brace)
            {
                is_template = false;

                Template tmp(template_name, types, template_args);
                templates.push_back(tmp);

                template_args.clear();
                types.clear();
                count = 0;
                atleast_one_brace = false;
                name_found = false;
            }
        }
        else if (validate(line, templ))
        {
            is_template = true;

            std::smatch match;
            std::regex_search(line, match, templ);

            std::string args = match[1];

            std::regex typename_regex("typename[ ]*");
            args = std::regex_replace(args, typename_regex, "");

            std::regex whitespace_regex("[ ]+");
            args = std::regex_replace(args, whitespace_regex, "");

            std::string str = "";

            for (auto &c : args)
            {
                if (c != ',')
                    str += c;
                else
                {
                    types.push_back(str);
                    str = "";
                }
            }

            types.push_back(str);
        }

        line_num++;
    }

    line_num = 1;

    std::vector<Template> generated_lines;

    for (auto &line : lines)
    {
        std::vector<std::string> matches;

        if (validate(matches, line, func_call, true))
        {
            for (auto match : matches)
            {
                std::smatch tmp_match;
                std::regex_search(match, tmp_match, func_call);

                std::string func_name = tmp_match[1];
                std::string func_types = tmp_match[2];
                std::string func_args = tmp_match[3];

                std::vector<std::string> func_type_args;
                std::string str = "";

                for (auto &c : func_types)
                {
                    if (c != ',')
                        str += c;
                    else
                    {
                        func_type_args.push_back(str);
                        str = "";
                    }
                }

                func_type_args.push_back(str);

                Template tmp;
                bool found = false;

                for (auto template_ : templates)
                {
                    if (template_.name == func_name)
                    {
                        tmp = template_;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    std::cout << "Error: Line " << line_num << ": "
                              << "Could not find template for function " << func_name << std::endl;
                    return 1;
                }

                int i = 0;

                for (auto &type : tmp.types)
                {
                    std::regex type_regex("\\b" + type + "\\b");

                    for (auto &l : tmp.content)
                        l = std::regex_replace(l, type_regex, func_type_args[i]);

                    i++;
                }

                std::string new_func_name = func_name + "_" + func_types;

                std::regex whitespace_regex("[ ]+");
                new_func_name = std::regex_replace(new_func_name, whitespace_regex, "");

                std::regex comma_regex(",");
                new_func_name = std::regex_replace(new_func_name, comma_regex, "");

                tmp.name = new_func_name;

                std::string new_tmp_name = tmp.name + "(" + func_args + ")";

                line = std::regex_replace(
                    line, std::regex(func_name + "<" + func_types + ">" + "\\(" + func_args + "\\)"), new_tmp_name);

                for (auto &l : tmp.content)
                    l = std::regex_replace(l, std::regex("\\b" + func_name + "\\b"), tmp.name);

                tmp.category = func_name;
                generated_lines.push_back(tmp);
            }
        }
        else if (validate(matches, line, struct_decl, true))
        {
            for (auto match : matches)
            {
                std::smatch tmp_match;
                std::regex_search(match, tmp_match, struct_decl);

                std::string struct_name = tmp_match[1];
                std::string struct_types = tmp_match[2];

                std::vector<std::string> struct_type_args;
                std::string str = "";

                for (auto &c : struct_types)
                {
                    if (c != ',')
                        str += c;
                    else
                    {
                        struct_type_args.push_back(str);
                        str = "";
                    }
                }

                struct_type_args.push_back(str);

                Template tmp;
                bool found = false;

                for (auto template_ : templates)
                {
                    if (template_.name == struct_name)
                    {
                        tmp = template_;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    std::cout << "Error: Line " << line_num << ": "
                              << "Could not find template for function " << struct_name << std::endl;
                    return 1;
                }

                int i = 0;

                for (auto &type : tmp.types)
                {
                    std::regex type_regex("\\b" + type + "\\b");

                    for (auto &l : tmp.content)
                        l = std::regex_replace(l, type_regex, struct_type_args[i]);

                    i++;
                }

                std::string new_struct_name = struct_name + "_" + struct_types;

                std::regex whitespace_regex("[ ]+");
                new_struct_name = std::regex_replace(new_struct_name, whitespace_regex, "");

                std::regex comma_regex(",");
                new_struct_name = std::regex_replace(new_struct_name, comma_regex, "");

                line = std::regex_replace(line, std::regex(struct_name + "<" + struct_types + ">"),
                                          "struct " + new_struct_name);
                tmp.name = new_struct_name;

                for (auto &l : tmp.content)
                    l = std::regex_replace(l, std::regex("\\b" + struct_name + "\\b"), tmp.name);

                tmp.category = struct_name;
                generated_lines.push_back(tmp);
            }
        }

        matches.clear();

        line_num++;
    }

    for (auto t : generated_lines)
    {
        std::cout << "Template: " << t.name << std::endl;
        std::cout << "Types: ";

        for (auto type : t.types)
        {
            std::cout << type << " ";
        }

        std::cout << std::endl;

        for (auto line : t.content)
        {
            std::cout << line << std::endl;
        }
    }

    std::cout << "Done!" << std::endl;

    std::cout << "Writing to file..." << std::endl;

    int i = 0;
    int j = 0;

    count = 0;
    atleast_one_brace = false;

    is_template = false;

    std::ofstream outfile;
    outfile.open(tmp);

    for (auto line : lines)
    {
        if (validate(line, templ))
        {
            is_template = true;
            i++;
        }

        if (is_template && line.find("{") != std::string::npos)
        {
            count++;
            atleast_one_brace = true;
        }

        if (is_template && line.find("}") != std::string::npos)
        {
            count--;
        }

        if (is_template && count == 0 && atleast_one_brace)
        {
            atleast_one_brace = false;
            i++;

            for (auto t : generated_lines)
            {
                if (t.category == templates[j].category)
                {
                    for (auto line : t.content)
                    {
                        outfile << line << std::endl;
                    }

                    outfile << std::endl;
                }
            }

            j++;
            is_template = false;
            continue;
        }

        if (i % 2 == 0)
            outfile << line << std::endl;
    }

    outfile.close();

    // std::cin.get();
    return 0;
}
