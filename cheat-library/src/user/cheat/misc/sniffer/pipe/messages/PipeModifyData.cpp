#include "pch-il2cpp.h"
#include "PipeModifyData.h"

	PipeModifyData::PipeModifyData() :
		modifyType(ModifyType::Unchanged), head({}), content({}), messageID(0)
	{ }

	void PipeModifyData::Write(PipeTransfer* transfer)
	{
		PipeMessage::Write(transfer);

		transfer->Write(modifyType);
		if (modifyType == ModifyType::Modified)
		{

			transfer->Write(messageID);
			transfer->Write(content);
			transfer->Write(head);
		}
	}

	void PipeModifyData::Read(PipeTransfer* transfer)
	{
		transfer->Read(modifyType);
		if (modifyType == ModifyType::Modified)
		{

			transfer->Read(messageID);
			transfer->Read(content);
			transfer->Read(head);
		}
	}
