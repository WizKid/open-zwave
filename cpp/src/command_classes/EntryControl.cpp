//-----------------------------------------------------------------------------
//
//	EntryControl.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_LOCK
//
//	Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include "command_classes/CommandClasses.h"
#include "command_classes/EntryControl.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "Notification.h"
#include "platform/Log.h"

#include "value_classes/ValueByte.h"

namespace OpenZWave
{
	namespace Internal
	{
		namespace CC
		{

			enum EntryControlCmd
			{
				EntryControlCmd_Notification = 0x01,
        EntryControlCmd_KeySupported_Get = 0x02,
        EntryControlCmd_KeySupported_Report = 0x03,
        EntryControlCmd_EventSupported_Get = 0x04,
        EntryControlCmd_EventSupported_Report = 0x05,
        EntryControlCmd_Configuration_Set = 0x06,
        EntryControlCmd_Configuration_Get = 0x07,
        EntryControlCmd_Configuration_Report = 0x08,
			};

//-----------------------------------------------------------------------------
// <EntryControl::RequestState>
// Request current state from the device
//-----------------------------------------------------------------------------
			bool EntryControl::RequestState(uint32 const _requestFlags, uint8 const _instance, Driver::MsgQueue const _queue)
			{
				bool requests = false;
				if ((_requestFlags & RequestFlag_Static) && HasStaticRequest(StaticRequest_Values))
				{
					requests = RequestValue(_requestFlags, EntryControlCmd_KeySupported_Get, _instance, _queue);
          requests = RequestValue(_requestFlags, EntryControlCmd_EventSupported_Get, _instance, _queue);
				}

				if (_requestFlags & RequestFlag_Dynamic)
				{
					requests |= RequestValue(_requestFlags, EntryControlCmd_Configuration_Get, _instance, _queue);
				}

				return requests;
			}

//-----------------------------------------------------------------------------
// <EntryControl::RequestValue>
// Request current value from the device
//-----------------------------------------------------------------------------
			bool EntryControl::RequestValue(uint32 const _requestFlags, uint16 const _what, uint8 const _instance, Driver::MsgQueue const _queue)
			{
        if (_what == EntryControlCmd_KeySupported_Get || _what == EntryControlCmd_EventSupported_Get || _what == EntryControlCmd_Configuration_Get)
        {
          Msg* msg = new Msg("EntryControlCmd_KeySupported_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId());
          msg->SetInstance(this, _instance);
          msg->Append(GetNodeId());
          msg->Append(2);
          msg->Append(GetCommandClassId());
          msg->Append(_what);
          msg->Append(GetDriver()->GetTransmitOptions());
          GetDriver()->SendMsg(msg, _queue);
          return true;
        }
				return false;
			}

//-----------------------------------------------------------------------------
// <EntryControl::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
			bool EntryControl::HandleMsg(uint8 const* _data, uint32 const _length, uint32 const _instance	/* = 1 */)
      {
        EntryControlCmd what = (EntryControlCmd) _data[0];
        if (what == EntryControlCmd_Notification)
        {
					uint8 sequenceNumber = _data[1];
					// uint8 dataType = _data[2] & 0x3;
					uint8 eventType = _data[3];
					uint8 eventDataLength = _data[4];

					string eventData(&_data[5], &_data[5] + eventDataLength);

					Notification* notification = new Notification(Notification::Type_EntryControl);
					notification->SetHomeAndNodeIds(GetHomeId(), GetNodeId());
					notification->SetSequenceNumber(sequenceNumber);
					notification->SetEvent(eventType);
					notification->SetEventData(eventData);
					GetDriver()->QueueNotification(notification);

          return true;
        }
        if (what == EntryControlCmd_KeySupported_Report)
        {
          return true;
        }
        if (what == EntryControlCmd_EventSupported_Report)
        {
					uint8 offset = 1;
					uint8 dataTypeSupportedBitmaskLength = _data[1] & 0x3;
					offset += 1 + dataTypeSupportedBitmaskLength;
					uint8 eventTypeSupportedBitmaskLength = _data[offset] & 0x1f;
					offset += 1 + eventTypeSupportedBitmaskLength;

					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Size_Minimum)))
					{
						value->OnValueRefreshed(_data[offset]);
						value->Release();
					}
					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Size_Maximum)))
					{
						value->OnValueRefreshed(_data[offset + 1]);
						value->Release();
					}
					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Timeout_Minimum)))
					{
						value->OnValueRefreshed(_data[offset + 2]);
						value->Release();
					}
					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Timeout_Maximum)))
					{
						value->OnValueRefreshed(_data[offset + 3]);
						value->Release();
					}

          return true;
        }
        if (what == EntryControlCmd_Configuration_Report)
        {
					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Size)))
					{
						value->OnValueRefreshed(_data[1]);
						value->Release();
					}
					if (Internal::VC::ValueByte* value = static_cast<Internal::VC::ValueByte*>(GetValue(_instance, ValueID_Index_EntryControl::Key_Cached_Timeout)))
					{
						value->OnValueRefreshed(_data[2]);
						value->Release();
					}
          return true;
        }
				return false;
			}

//-----------------------------------------------------------------------------
// <EntryControl::SetValue>
// Set the lock's state
//-----------------------------------------------------------------------------
			bool EntryControl::SetValue(Internal::VC::Value const& _value)
			{
				return false;
			}

//-----------------------------------------------------------------------------
// <EntryControl::CreateVars>
// Create the values managed by this command class
//-----------------------------------------------------------------------------
			void EntryControl::CreateVars(uint8 const _instance)
			{
				if (Node* node = GetNodeUnsafe())
				{
					node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Size, "Key Cached Size", "", false, false, 0, 0);
					node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Size_Minimum, "Key Cached Size Minimum", "", true, false, 0, 0);
          node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Size_Maximum, "Key Cached Size Maximum", "", true, false, 0xff, 0);
					node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Timeout, "Key Cached Timeout", "", false, false, 0, 0);
          node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Timeout_Minimum, "Key Cached Timeout Minimum", "", true, false, 0, 0);
          node->CreateValueByte(ValueID::ValueGenre_System, GetCommandClassId(), _instance, ValueID_Index_EntryControl::Key_Cached_Timeout_Maximum, "Key Cached Timeout Maximum", "", true, false, 0xff, 0);
				}
			}
		} // namespace CC
	} // namespace Internal
} // namespace OpenZWave