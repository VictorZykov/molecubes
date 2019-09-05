using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MSFPlotTool
{
    public class MCube
    {
        public int trgclass;
        public int id;
        public List<MCommand> commands = new List<MCommand>();
        
        public MCube()
        { }

        public override string ToString()
        {
            return "Cube #" + id;
        }

    }
}
