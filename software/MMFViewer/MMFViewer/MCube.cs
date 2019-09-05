using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MMFViewer
{
    class MCube
    {
        public int id;
        public int cubeClass;
        public int numLinks;
        public List<MLink> links = new List<MLink>();

        public MCube() { }

        public override string ToString()
        {
            return "ID:0x" + id.ToString("X2") + " Class:0x" + cubeClass.ToString("X2");
        }
    }
}
