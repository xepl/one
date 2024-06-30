#include "xepl.hpp"

int main ( int, char**, char** )
{
    XEPL::Cortex cortex ( "brain", std::cout );


	// Add an XML Keyword to Print XML result string
    cortex.Register_Keyword ( "Print", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
	{
       std::cout << *_param << std::endl;
		_neuron->Process_Inner_Genes ( _call_gene );
	} );
    cortex.Execute_As_Xml("<Print>Hello, World!</Print>");



	// Your first RNA.operator - just adds a space
	cortex.Register_Operator ( "space", [] ( XEPL::Script* _script, XEPL::Cord* _rhs )
	{
		_script->value->push_back ( ' ' );
		if ( _rhs )
			_script->value->append ( *_rhs );
	} );
	cortex.Execute_As_Rna("'RNA Operator'.space('Xepl')");


	// Demonstrate adding a Method to a Neuron
	{
		// We need a Keyword to add the XML Method to a Neuron
		cortex.Register_Keyword ( "Method", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* )
		{
			XEPL::String name;
			if ( _call_gene->Trait_Get("name", &name ) )
				_neuron->Register_Method ( &name, &XEPL::Nucleus::Method_Execute, _call_gene );
		} );

		// Now, call the keyword to Add a Method to the current Neuron (the brain)
		cortex.Execute_As_Xml(
			"<Method name='Speak'>"
				"<Print>{'Speak'.space(to)}</Print>"
			"</Method>");

		// Now ... call the method on the Neuron
		cortex.Execute_As_Xml("<brain.Speak to='method param'/>");
	}


	// Demonstrates ShortTerm values, watch the 'iter' value
	{
		cortex.Register_Keyword ( "Repeat", [] ( XEPL::Neuron* _neuron, XEPL::Gene* _call_gene, XEPL::String* _param )
		{
			long repeat_long = std::strtol ( _param->c_str(), nullptr, 0 );
			if ( repeat_long < 1 || !_call_gene->inner_genes )
				return;

			XEPL::String count;
			while ( repeat_long-- )
			{
				count.assign(std::to_string(repeat_long));
				XEPL::ShortTerms hold("iter",&count);
				_neuron->Process_Inner_Genes ( _call_gene );
			}
		} );
		cortex.Execute_As_Xml(
			"<Repeat>10"
				"<Print>{'countdown'.space(iter)}"
					"<Repeat>{iter}"
						"<Print>{'  inner'.space(iter)}</Print>"
					"</Repeat>"
				"</Print>"
			"</Repeat>");
	}

  	return 0;

	// XEPL will clean up and verify all memory has been returned
}