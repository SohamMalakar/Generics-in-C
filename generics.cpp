#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

struct Template
{
    string name;
    vector<string> types;
    vector<string> content;
};

struct Generated
{
    string name;
    string template_name;
    vector<string> types;
    vector<string> content;
};

bool reg_match(vector<string> &matches, string str, regex reg)
{
    bool is_match = false;

    sregex_iterator current_match(str.begin(), str.end(), reg);
    sregex_iterator last_match;

    while (current_match != last_match)
    {
        smatch match = *current_match;
        matches.push_back(match.str());
        is_match = true;
        current_match++;
    }

    return is_match;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <file>" << '\n';
        return 1;
    }

    ifstream file;

    file.open(argv[1], ios_base::binary);

    if (!file.is_open())
    {
        cout << "Could not open file " << argv[1] << '\n';
        return 1;
    }

    string name(argv[1]);
    string tmp = name.substr(0, name.find_last_of(".")) + "_gen.c";

    string line;
    vector<string> lines;

    while (getline(file, line))
        lines.push_back(line);

    file.close();

    regex templ("template[ ]*<(.*?)>");
    regex func_def("([\\w_]+)\\((.*?)\\)");
    regex func_call("([\\w_]+)<(.*?)>\\((.*?)\\)");
    regex struct_def("struct[ ]*([\\w_]+)");
    regex struct_decl("([\\w_]+)<(.*?)>[ ]*");

    // Find templates
    vector<Template> templates;
    Template tmpl;
    bool is_template = false;
    bool atleast_one_brace = false;
    bool name_found = false;
    int count = 0;

    for (auto l : lines)
    {
        smatch match;

        if (is_template)
        {
            if (!name_found && (regex_search(l, match, func_def) || regex_search(l, match, struct_def)))
            {
                tmpl.name = match[1];
                name_found = true;
            }

            tmpl.content.push_back(l);

            if (l.find("{") != string::npos)
                atleast_one_brace = true;

            count += std::count(l.begin(), l.end(), '{') - std::count(l.begin(), l.end(), '}');

            if (count == 0 && atleast_one_brace)
            {
                templates.push_back(tmpl);
                is_template = false;
                atleast_one_brace = false;
                name_found = false;

                tmpl.types.clear();
                tmpl.content.clear();
            }
        }
        else if (regex_search(l, match, templ))
        {
            is_template = true;

            string args = match[1];
            args = regex_replace(args, regex("typename[ ]*"), "");
            args = regex_replace(args, regex("[ ]*"), "");

            std::string str = "";

            for (auto c : args)
            {
                if (c != ',')
                    str += c;
                else
                {
                    tmpl.types.push_back(str);
                    str = "";
                }
            }

            tmpl.types.push_back(str);
        }
    }

    vector<Generated> gens;
    bool has_generate;

    is_template = false;
    atleast_one_brace = false;
    count = 0;

    vector<string> new_lines;
    unordered_set<string> generated_names;

    do
    {
        has_generate = false;

        for (auto &l : lines)
        {
            vector<string> matches;

            // Skip the templates
            if (regex_search(l, templ))
                is_template = true;

            if (is_template)
            {
                if (l.find("{") != string::npos)
                    atleast_one_brace = true;

                count += std::count(l.begin(), l.end(), '{') - std::count(l.begin(), l.end(), '}');

                if (count == 0 && atleast_one_brace)
                {
                    is_template = false;
                    atleast_one_brace = false;
                }

                continue;
            }

            if (reg_match(matches, l, func_call))
            {
                has_generate = true;

                // Replace the function call with the C style function call
                // e.g. foo<int, char>() -> foo_int_char()
                // And add the generated function to the list of generated functions
                // Do not generate the function if it is already in the list

                for (auto match : matches)
                {
                    smatch sm;
                    regex_search(match, sm, func_call);

                    string func_name = sm[1];
                    string func_types = sm[2];
                    string func_args = sm[3];

                    vector<string> func_type_list;
                    string func_type_str = "";

                    for (auto c : func_types)
                    {
                        if (c != ',')
                            func_type_str += c;
                        else
                        {
                            func_type_list.push_back(func_type_str);
                            func_type_str = "";
                        }
                    }

                    func_type_list.push_back(func_type_str);

                    bool has_found = false;
                    Generated gen;

                    for (auto t : templates)
                    {
                        if (t.name == func_name)
                        {
                            has_found = true;

                            gen.name = func_name;
                            gen.template_name = func_name;
                            gen.content = t.content;
                            gen.types = t.types;
                            break;
                        }
                    }

                    if (!has_found)
                    {
                        cout << "Could not find template for function " << func_name << '\n';
                        return 1;
                    }

                    int i = 0;

                    for (auto &type : gen.types)
                    {
                        for (auto &l : gen.content)
                            l = regex_replace(l, regex("\\b" + type + "\\b"), func_type_list[i]);
                        i++;
                    }

                    string new_func_name = func_name + "_" + func_types;

                    new_func_name = regex_replace(new_func_name, regex("[ ]*"), "");
                    new_func_name = regex_replace(new_func_name, regex(","), "");

                    gen.name = new_func_name;

                    l = regex_replace(l, regex(func_name + "<" + func_types + ">" + "\\(" + func_args + "\\)"),
                                      new_func_name + "(" + func_args + ")");

                    for (auto &li : gen.content)
                        li = regex_replace(li, regex("\\b" + func_name + "\\b"), new_func_name);

                    bool already_exists = false;

                    // Check if the function is already generated in the previous iteration
                    for (auto g : gens)
                        generated_names.insert(g.name);

                    already_exists = generated_names.find(new_func_name) != generated_names.end();

                    if (!already_exists)
                        gens.push_back(gen);
                }

                matches.clear();
            }

            if (reg_match(matches, l, struct_decl))
            {
                has_generate = true;

                // Replace the struct declaration with the C style struct declaration
                // e.g. foo<int, char> -> foo_int_char
                // And add the generated struct to the list of generated structs
                // Do not generate structs that are already defined

                for (auto match : matches)
                {
                    smatch sm;
                    regex_search(match, sm, struct_decl);

                    string struct_name = sm[1];
                    string struct_types = sm[2];

                    vector<string> struct_type_list;
                    string struct_type_str = "";

                    for (auto c : struct_types)
                    {
                        if (c != ',')
                            struct_type_str += c;
                        else
                        {
                            struct_type_list.push_back(struct_type_str);
                            struct_type_str = "";
                        }
                    }

                    struct_type_list.push_back(struct_type_str);

                    bool has_found = false;
                    Generated gen;

                    for (auto t : templates)
                    {
                        if (t.name == struct_name)
                        {
                            has_found = true;

                            gen.name = struct_name;
                            gen.template_name = struct_name;
                            gen.content = t.content;
                            gen.types = t.types;
                            break;
                        }
                    }

                    if (!has_found)
                    {
                        cout << "Could not find template for structure " << struct_name << '\n';
                        return 1;
                    }

                    int i = 0;

                    for (auto &type : gen.types)
                    {
                        for (auto &l : gen.content)
                            l = regex_replace(l, regex("\\b" + type + "\\b"), struct_type_list[i]);
                        i++;
                    }

                    string new_struct_name = struct_name + "_" + struct_types;

                    new_struct_name = regex_replace(new_struct_name, regex("[ ]*"), "");
                    new_struct_name = regex_replace(new_struct_name, regex(","), "");

                    gen.name = new_struct_name;

                    l = regex_replace(l, regex(struct_name + "<" + struct_types + ">"), "struct " + new_struct_name);

                    for (auto &li : gen.content)
                        li = regex_replace(li, regex("\\b" + struct_name + "\\b"), new_struct_name);

                    bool already_exists = false;

                    // Check if the structure is already generated in the previous iteration
                    for (auto g : gens)
                        generated_names.insert(g.name);

                    already_exists = generated_names.find(new_struct_name) != generated_names.end();

                    if (!already_exists)
                        gens.push_back(gen);
                }

                matches.clear();
            }
        }

        string tmpl_name;

        // Add the generated functions and structs to the lines
        for (auto l : lines)
        {
            if (is_template)
            {
                smatch match;

                if (!name_found && (regex_search(l, match, func_def) || regex_search(l, match, struct_def)))
                {
                    tmpl_name = match[1];
                    name_found = true;
                }

                if (l.find("{") != string::npos)
                    atleast_one_brace = true;

                count += std::count(l.begin(), l.end(), '{') - std::count(l.begin(), l.end(), '}');

                if (count == 0 && atleast_one_brace)
                {
                    is_template = false;
                    atleast_one_brace = false;
                    name_found = false;

                    new_lines.push_back(l);

                    // Add the generated functions and structs to the lines
                    for (auto g : gens)
                        if (g.template_name == tmpl_name)
                            for (auto ln : g.content)
                                new_lines.push_back(ln);

                    continue;
                }
            }
            else if (regex_search(l, templ))
                is_template = true;

            new_lines.push_back(l);
        }

        lines.clear();
        lines = new_lines;
        new_lines.clear();
        gens.clear();

    } while (has_generate);

    // Remove templates
    for (auto l : lines)
    {
        if (is_template)
        {
            if (l.find("{") != string::npos)
                atleast_one_brace = true;

            count += std::count(l.begin(), l.end(), '{') - std::count(l.begin(), l.end(), '}');

            if (count == 0 && atleast_one_brace)
            {
                is_template = false;
                atleast_one_brace = false;
            }
        }
        else if (regex_search(l, templ))
            is_template = true;
        else
            new_lines.push_back(l);
    }

    lines.clear();
    lines = new_lines;
    new_lines.clear();

    ofstream out_file(tmp);

    // Write the lines to the generated file
    for (auto l : lines)
        out_file << l << '\n';

    out_file.close();

    return 0;
}
