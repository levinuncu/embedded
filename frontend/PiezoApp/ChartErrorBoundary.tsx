// import React from 'react';
// import { View, Text, StyleSheet } from 'react-native';

// interface Props {
//     children: React.ReactNode;
//     height: number;
// }

// interface State {
//     hasError: boolean;
// }

// export class ChartErrorBoundary extends React.Component<Props, State> {
//     constructor(props: Props) {
//         super(props);
//         this.state = { hasError: false };
//     }

//     static getDerivedStateFromError(): State {
//         return { hasError: true };
//     }

//     componentDidCatch(error: any) {
//         console.warn('Chart render error caught:', error);
//         setTimeout(() => this.setState({ hasError: false }), 2000);
//     }

//     render(): React.ReactNode {
//         if (this.state.hasError) {
//             return (
//                 <View style={[styles.placeholder, { height: this.props.height }]}>
//                     <Text style={styles.text}>rendering...</Text>
//                 </View>
//             );
//         }
//         return this.props.children;
//     }
// }

// const styles = StyleSheet.create({
//     placeholder: {
//         justifyContent: 'center',
//         alignItems: 'center',
//         backgroundColor: '#f7f7f7',
//         borderRadius: 6,
//     },
//     text: { fontSize: 12, color: '#aaa' },
// });