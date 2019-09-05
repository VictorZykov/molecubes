using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MSFPlotTool
{
    public class MCommand
    {
        public int timestamp;
        public int bus;
        public int header;
        public int trgclass;
        public int id; // addr
        public int length;
        public int instruction;
        public byte[] param;
        public int chksum;

        public MCommand()
        { }

        public string getParams()
        {
            string outstr = "";
            for(int i=0;i<length; i++)
            {
                outstr += param[i].ToString("X2") + " ";
            }
            return outstr;
        }
    }
}
