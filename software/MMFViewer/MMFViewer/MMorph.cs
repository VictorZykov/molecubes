using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace MMFViewer
{
    class MMorph
    {
        int cubecount;
        public List<MCube> cubes = new List<MCube>();
        int statecount;
        public List<MState> states = new List<MState>();

        public MMorph() { }

        public void openMMFile(string filename)
        {
            FileStream strm;
            strm = new FileStream(filename, FileMode.Open, FileAccess.Read);
            BinaryReader br = new BinaryReader(strm);
            
            cubecount = br.ReadByte();
            
            for (int cubecounter = 0; cubecounter < cubecount; cubecounter++)
            {
                MCube newCube = new MCube();
                newCube.id = br.ReadByte();
                newCube.cubeClass = br.ReadByte();
                newCube.numLinks = br.ReadByte();
                for (int linkcounter = 0; linkcounter < newCube.numLinks; linkcounter++)
                {
                    MLink newLink = new MLink();
                    newLink.selfID = br.ReadByte();
                    newLink.selfClass = br.ReadByte();
                    newLink.selfSide = br.ReadByte();
                    newLink.selfOrient = br.ReadByte();
                    newLink.neighborID = br.ReadByte();
                    newLink.neighborClass = br.ReadByte();
                    newLink.neighborSide = br.ReadByte();
                    newCube.links.Add(newLink);
                }
                cubes.Add(newCube);
            }

            byte chksum = br.ReadByte(); // foolishly throw away checksum
            if(chksum != calcCubesChecksum(cubecount, cubes))
                throw new InvalidOperationException("Calculated Cube Checksum Does Not Match File");
            

            statecount = br.ReadInt32();

            for (int statecounter = 0; statecounter < statecount; statecounter++)
            {
                MState newState = new MState();
                newState.id = br.ReadByte();
                newState.cubeClass = br.ReadByte();
                newState.timestamp = br.ReadInt32();
                newState.channel = br.ReadByte();
                newState.value = br.ReadInt32();
                states.Add(newState);
            }

            chksum = br.ReadByte(); // foolishly throw away checksum

            // Read vs Calculated State Checksums aren't working correctly, bug fixed on
            // controller side which might improve this
            Console.WriteLine(chksum + " " + calcStateChecksum(statecount, states));
            //if (chksum != calcStateChecksum(statecount, states))
            //    throw new InvalidOperationException("Calculated Cube Checksum Does Not Match File");

            strm.Close();
        }

        private byte calcCubesChecksum(int cubecount, List<MCube> cubes)
        {
            byte sum = 0;
            sum += (byte)cubecount;
            foreach (MCube cube in cubes)
            {
                sum += (byte)cube.id;
                sum += (byte)cube.cubeClass;
                sum += (byte)cube.numLinks;
                foreach (MLink link in cube.links)
                {
                    sum += (byte)link.selfID;
                    sum += (byte)link.selfClass;
                    sum += (byte)link.selfSide;
                    sum += (byte)link.selfOrient;
                    sum += (byte)link.neighborID;
                    sum += (byte)link.neighborClass;
                    sum += (byte)link.neighborSide;
                }
            }
            return sum;
        }

        private byte calcStateChecksum(int statecount, List<MState> states)
        {
            byte sum = 0;
            sum += (byte)statecount;
            foreach (MState state in states)
            {
                sum += (byte)state.id;
                sum += (byte)state.cubeClass;
                sum += (byte)state.timestamp;
                sum += (byte)state.channel;
                sum += (byte)state.value;
            }
            return sum;
        }
    }
}
