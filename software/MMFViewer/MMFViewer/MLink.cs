using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MMFViewer
{
    class MLink
    {
        public int selfID;
        public int selfClass;
        public int selfSide;
        public int selfOrient;
        public int neighborID;
        public int neighborClass;
        public int neighborSide;

        public MLink() { }

        public override string ToString()
        {
            return "0x" + selfID.ToString("X2") + ":0x" + selfClass.ToString("X2") + " <--> " + "0x" + neighborID.ToString("X2") + ":0x" + neighborClass.ToString("X2");
        }

    }
}
