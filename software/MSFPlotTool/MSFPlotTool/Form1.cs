using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using ZedGraph;

namespace MSFPlotTool
{
    public partial class MSFPlotTool : Form
    {
        MSequence msf;

        public MSFPlotTool()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            msf = new MSequence();
            CreateGraph(zg1);
            
        }

        private void CreateGraph(ZedGraphControl zgc)
        {
            GraphPane myPane = zgc.GraphPane;
            
            // Set the titles and axis labels
            myPane.Title.Text = "MSF Visualization: " + Path.GetFileName(ofd1.FileName);
            myPane.XAxis.Title.Text = "Sequence Time (msec)";
            myPane.YAxis.Title.Text = "Joint Angle (degrees)";
            
            // Generate a blue curve with circle symbols, and "My Curve 2" in the legend
            //LineItem myCurve = myPane.AddCurve("My Curve", list, Color.Blue, SymbolType.Circle);
            // Fill the area under the curve with a white-red gradient at 45 degrees
            //myCurve.Line.Fill = new Fill(Color.White, Color.Red, 45F);
            // Make the symbols opaque by filling them with white
            //myCurve.Symbol.Fill = new Fill(Color.White);

            updateDisplayedSequenceMovements(zgc);

            // Fill the axis background with a color gradient
           // myPane.Chart.Fill = new Fill(Color.White, Color.LightBlue, 45F);

            // Fill the pane background with a color gradient
            myPane.Fill = new Fill(Color.White, Color.FromArgb(220, 220, 255), 45F);

            // Calculate the Axis Scale Ranges
            zgc.AxisChange();
            myPane.YAxis.Scale.Min = 0.0;
            myPane.YAxis.Scale.Max = 360.0;
            // Force a redraw
            zgc.Invalidate();
        }

        private void openMSFToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ofd1.ShowDialog() == DialogResult.Cancel)
                return;
            
            
            try
            {
                msf = new MSequence(ofd1.FileName);
            }
            catch (Exception)
            {
                MessageBox.Show("Error opening file", "File Error",
                                 MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }

            updateSequenceInfoLabel();
            buildCubeList();
            buildCommandList();
            CreateGraph(zg1);
            pictureBox1.Visible = false;
            zg1.Visible = true;
        }

        private void buildCubeList()
        {
            checkedListBox1.Items.Clear();
            foreach (MCube cube in msf.cubes.Values)
            {
                checkedListBox1.Items.Add(cube, CheckState.Checked);
            }
        }

        private void buildCommandList()
        {
            listView1.Items.Clear();
            foreach (MCommand cmd in msf.commands)
            {
                if (cmd.instruction == 0x15)
                {
                    double pos, velo;
                    pos = (cmd.param[0] | (cmd.param[1] << 8)) / 10.0;
                    velo = (cmd.param[2] | (cmd.param[3] << 8));
                    listView1.Items.Add(new ListViewItem(new string[] { 
                    cmd.timestamp.ToString(), 
                    cmd.bus.ToString("X2"),
                    cmd.trgclass.ToString("X2"),
                    cmd.id.ToString("X2"),
                    cmd.length.ToString("X2"),
                    cmd.instruction.ToString("X2"),
                    cmd.getParams(),
                    pos.ToString(),
                    velo.ToString()
                    }));
                }
                else
                {
                    listView1.Items.Add(new ListViewItem(new string[] { 
                    cmd.timestamp.ToString(), 
                    cmd.bus.ToString("X2"),
                    cmd.trgclass.ToString("X2"),
                    cmd.id.ToString("X2"),
                    cmd.length.ToString("X2"),
                    cmd.instruction.ToString("X2"),
                    cmd.getParams()
                    }));
                }
                
            }
        }

        private void updateSequenceInfoLabel()
        {
            string labelval = "Name: " + msf.sequenceName + "\n";
            labelval += "Topology: " + msf.topologyHash + "\n";
            labelval += "Commands: " + msf.commandCount + "\n";
            labelval += "Size: " + msf.size + "\n";
            label1.Text = labelval;
        }

        private void updateDisplayedSequenceMovements(ZedGraphControl zgc)
        {
            GraphPane myPane = zgc.GraphPane;
            myPane.CurveList.Clear();
            foreach (MCube cube in msf.cubes.Values)
            {
                PointPairList ang_list = new PointPairList();
                PointPairList velo_list = new PointPairList();
                if (checkedListBox1.GetItemChecked(checkedListBox1.Items.IndexOf(cube)))
                {
                    foreach (MCommand cmd in cube.commands)
                    {
                        
                        Console.WriteLine(cmd.id + " " + cmd.instruction);
                        // if cmds are correct type, interpret params and generate a PointPairList
                        if (cmd.instruction == 0x15) // joint goto command
                        {
                            double angle = (double)(cmd.param[0] | (cmd.param[1] << 8)) / 10.0;
                            double velo = (double)(cmd.param[2] | (cmd.param[3] << 8)) / 10.0;
                            Console.WriteLine(cmd.timestamp + " " + angle + " " + velo);
                            ang_list.Add(cmd.timestamp, angle);
                            velo_list.Add(cmd.timestamp, velo);
                        }
                        
                    }
                    LineItem myCurve = myPane.AddCurve(cube.ToString(), ang_list, getColor(cube.id), getSymbol(cube.id));
                }
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox a = new AboutBox();
            a.Show();
        }

        private void checkedListBox1_Click(object sender, EventArgs e)
        {
            CreateGraph(zg1);
        }

        private Color getColor(int i)
        {
            Color[] c =
            {
            Color.Black,
            Color.DodgerBlue,
            Color.Purple,
            Color.Green,
            Color.GreenYellow,
            Color.Yellow,
            Color.Orange,
            Color.OrangeRed,
            Color.Red,
            Color.Brown,
            Color.Maroon,
            Color.OliveDrab
            };
            return c[i % c.Length];
        }

        private SymbolType getSymbol(int i)
        {
            SymbolType[] s =
            {
            SymbolType.Circle,
            SymbolType.Diamond,
            SymbolType.Plus,
            SymbolType.Square,
            SymbolType.Star,
            SymbolType.Triangle,
            SymbolType.TriangleDown,
            SymbolType.VDash,
            SymbolType.XCross
            };
            return s[i % s.Length];
        }
    }
}
