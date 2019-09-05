using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MMFViewer
{
    public partial class Form1 : Form
    {
        MMorph mmorph;
        public Form1()
        {
            InitializeComponent();
            mmorph = new MMorph();
        }

        private void renderMorphology()
        {
            cubeListBox.Items.Clear();
            stateListBox.Items.Clear();

            foreach (MCube cube in mmorph.cubes)
            {
                cubeListBox.Items.Add(cube);
            }

            foreach (MState state in mmorph.states)
            {
                stateListBox.Items.Add(state);
            }
        }

        private void openMMFFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ofd1.ShowDialog() == DialogResult.Cancel)
                return;

            try
            {
                mmorph.openMMFile(ofd1.FileName);
            }
            catch (Exception)
            {
                MessageBox.Show("Error opening file", "File Error",
                                 MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            renderMorphology();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void cubeListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            MCube cube = (MCube)cubeListBox.SelectedItem;

            textBox3.Text = cube.id.ToString("X2");
            textBox2.Text = cube.cubeClass.ToString("X2");
            textBox1.Text = cube.numLinks.ToString();

            linkListBox.Items.Clear();

            foreach (MLink link in cube.links)
            {
                linkListBox.Items.Add(link);
            }
            clearListDetails();
        }

        private void clearListDetails()
        {
            textBox4.Text = String.Empty;
            textBox5.Text = String.Empty;
            textBox6.Text = String.Empty;
            textBox7.Text = String.Empty;
            textBox8.Text = String.Empty;
            textBox9.Text = String.Empty;
            textBox10.Text = String.Empty;
        }

        private void linkListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            MLink link = (MLink)linkListBox.SelectedItem;

            textBox4.Text = link.selfID.ToString("X2");
            textBox5.Text = link.selfClass.ToString("X2");
            textBox6.Text = link.selfSide.ToString("X2");
            textBox7.Text = link.selfOrient.ToString("X2");
            textBox8.Text = link.neighborID.ToString("X2");
            textBox9.Text = link.neighborClass.ToString("X2");
            textBox10.Text = link.neighborSide.ToString("X2");
        }

        private void stateListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            MState state = (MState)stateListBox.SelectedItem;

            textBox11.Text = state.id.ToString("X2");
            textBox12.Text = state.cubeClass.ToString("X2");
            textBox13.Text = state.timestamp.ToString();
            textBox14.Text = state.channel.ToString("X2");
            textBox15.Text = state.value.ToString();
        }

    }
}
