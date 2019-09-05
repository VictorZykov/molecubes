using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Collections;

namespace MSFPlotTool
{
    public class MSequence
    {
        public string sequenceName;
        public int topologyHash;
        public int commandCount;
        public int size;
        public List<MCommand> commands = new List<MCommand>();
        public Dictionary<int, MCube> cubes = new Dictionary<int, MCube>();
        int chksum;

        public MSequence()
        {
        }

        public MSequence(string filename)
        {
            openSequenceFile(filename);
        }



        public void openSequenceFile(string filename)
        {
            FileStream strm;
            strm = new FileStream(filename, FileMode.Open, FileAccess.Read);
            BinaryReader br = new BinaryReader(strm);

            sequenceName = System.Text.ASCIIEncoding.ASCII.GetString(br.ReadBytes(25));
            sequenceName = sequenceName.Substring(0, sequenceName.IndexOf("\0"));
            topologyHash = br.ReadInt32();
            commandCount = br.ReadInt32();
            size = br.ReadInt32();
            chksum = br.ReadByte(); // checksum

            for (int cmdcounter = 0; cmdcounter < commandCount; cmdcounter++)
            {
                MCommand newCommand = new MCommand();
                newCommand.timestamp = br.ReadInt32();
                newCommand.bus = br.ReadByte();
                newCommand.header = br.ReadByte(); // header 0xff
                newCommand.trgclass = br.ReadByte();
                newCommand.id = br.ReadByte();
                newCommand.length = br.ReadByte();
                newCommand.instruction = br.ReadByte();
                newCommand.param = br.ReadBytes(newCommand.length);
                newCommand.chksum = br.ReadByte();
                commands.Add(newCommand);

                if (cubes.ContainsKey(newCommand.id))
                {
                    MCube c = cubes[newCommand.id];
                    c.commands.Add(newCommand);
                }
                else
                {
                    MCube c = new MCube();
                    c.id = newCommand.id;
                    c.trgclass = newCommand.trgclass;
                    c.commands.Add(newCommand);
                    cubes.Add(c.id, c);
                }
            }
        }
    }
}
