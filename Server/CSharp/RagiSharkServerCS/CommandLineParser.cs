using System;
using System.Collections.Generic;
using System.Linq;

namespace RagiSharkServerCS
{
    class CommandLineArg
    {
        public string Key { get; set; }
        public object Value { get; set; }
    }

    class CommandLineArgDefinition
    {
        public string[] Keys { get; set; }
        public Func<string, object> Converter { get; set; }
    }

    class CommandLineParser
    {
        public static IEnumerable<CommandLineArg> Parse(string[] args, IEnumerable<CommandLineArgDefinition> definitions)
        {
            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                var definition = definitions.FirstOrDefault(x => x.Keys?.Contains(arg) == true);
                bool hasNext = i + 1 < args.Length;
                string value = hasNext ? args[i + 1] : "";

                if (hasNext)
                {
                    i++;

                    if (definition == null)
                    {
                        continue;
                    }

                    if (definition.Converter != null)
                    {
                        object convertedValue = definition.Converter(value);
                        yield return new CommandLineArg { Key = definition.Keys[0], Value = convertedValue };
                    }
                    else
                    {
                        yield return new CommandLineArg { Key = definition.Keys[0], Value = value };
                    }
                    continue;
                }

                yield return new CommandLineArg { Key = definition.Keys[0] };
            }
        }
    }
}