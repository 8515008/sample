using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace client2
{
    class Program
    {
        [StructLayout(LayoutKind.Explicit, Size = 16)]
        public class EVENT_DESCRIPTOR
        {
            [FieldOffset(0)]
            ushort Id = 1;
            [FieldOffset(2)]
            byte Version = 0;
            [FieldOffset(3)]
            byte Channel = 0;
            [FieldOffset(4)]
            byte Level = 4;
            [FieldOffset(5)]
            byte Opcode = 0;
            [FieldOffset(6)]
            ushort Task = 0;
            [FieldOffset(8)]
            long Keyword = 0;
        }

        [StructLayout(LayoutKind.Explicit, Size = 16)]
        public struct EventData
        {
            [FieldOffset(0)]
            internal UInt64 DataPointer;
            [FieldOffset(8)]
            internal uint Size;
            [FieldOffset(12)]
            internal int Reserved;
        }

        //...

        static void startService()
        {
            Guid webCleintTrigger = new Guid(0xb52c6d34, 0x59c9, 0x4dfc, 0x80, 0xcb, 0x3b, 0x65, 0x2, 0xf5, 0xd5, 0xcc);

            long handle = 0;
            uint output = EventRegister(ref webCleintTrigger, IntPtr.Zero, IntPtr.Zero, ref handle);

            //This is what is returned:
            //output = 0 <- Good 
            //handle = 65537  <- Good handle?

            bool success = false;

            if (output == 0)
            {
                //Create event descriptor
                EVENT_DESCRIPTOR desc = new EVENT_DESCRIPTOR();

                //Write the event
                unsafe
                {
                    uint writeOutput = EventWrite(handle, ref desc, 0, null);
                    success = writeOutput == 0;

                    EventUnregister(handle);
                }

            }
        }

        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern uint EventRegister(ref Guid guid, [Optional] IntPtr EnableCallback, [Optional] IntPtr CallbackContext, [In][Out] ref long RegHandle);

        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern unsafe uint EventWrite(long RegHandle, ref EVENT_DESCRIPTOR EventDescriptor, uint UserDataCount, EventData* UserData);

        [DllImport("Advapi32.dll", SetLastError = true)]
        public static extern uint EventUnregister(long RegHandle);

        static void Main(string[] args)
        {
            startService();
        }
    }
}
