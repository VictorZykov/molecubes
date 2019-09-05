using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MMFViewer
{
    class MState
    {
        public int id;
        public int cubeClass;
        public int timestamp;
        public int channel;
        public int value;

        public MState() { }

        public override string ToString()
        {
            return "TS:"+timestamp + " ID:0x" + id.ToString("X2") + " Class:0x" + cubeClass.ToString("X2");
        }
    }
}
